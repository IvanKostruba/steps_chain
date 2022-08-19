#pragma once

#include "util.h"
#include "marshalling_helper.h"

#include <array>
#include <tuple>

namespace steps_chain {

using namespace helpers;

// A class implementing resumable serial sequence of steps, without forks.
//
// Each step must be a callable (function, static method, lambda or functor) that takes
// a single argument and returns a single value.
//
// Return value can be std::optional in that case returning std::nullopt will break
// the chain execution and current argument will stay the same.
//
// Types of arguments and return values must be constructible from std::string and
// provide a serialize() method that returns a std::string.
//
// Initial step is initialized with serialized arguments, each step output is
// passed to the next step as an input.
//
// On each step current status can be serialized and stored if needed, alternatively
// it is possible to run until an exception occurs or final step is reached and then
// serialize, if needed.

template <typename... Steps>
class StepsChain
{
public:
    static_assert(are_chainable<Steps...>(),
                  "Return type of the previous function must be the same as argument type of the next." \
                  "If you use optional return type, next function argument should not be optional");
    StepsChain(Steps... steps) : _steps{steps...}, _current{0} {}

    // Run all remaining steps, beginnig with given index.
    bool run(std::string parameters, uint8_t begin_idx = 0) {
        if (begin_idx >= sizeof...(Steps)) {
            return false;
        }
        initialize(std::move(parameters), begin_idx);
        return execute_from(begin_idx);
    }

    // Just initializer, intended to be used in pair with advance()
    bool initialize(std::string parameters, uint8_t current_idx = 0) {
        _current = current_idx;
        deserialize_arguments(std::move(parameters));
        return current_idx < sizeof...(Steps);
    }

    bool advance() {
        if (_current >= sizeof...(Steps)) {
            return false;
        }
        return execute_current();
    }

    // Run all remaining steps, beginning with current index.
    bool resume() {
        if (_current >= sizeof...(Steps)) {
            return false;
        }
        return execute_from(_current);
    }

    // Get step index and serialized arguments for current step so that they can be stored.
    // If final step was executed, final result will be returned.
    std::tuple<uint8_t, std::string> get_current_state() const {
        return std::make_tuple(_current, serialize_current_args());
    }

    bool is_finished() const { return _current >= sizeof...(Steps); }

private:
    // ----- Instantiate callable invokers -----

    template <uint8_t idx>
    static constexpr auto make_invoker() {
        return [](steps_type& steps, current_arguments_type& data) -> uint8_t {
            using argument_type = std::decay_t<
                typename signature<std::tuple_element_t<idx, steps_type>>::arg_type>;
            using return_type = std::decay_t<
                typename signature<std::tuple_element_t<idx, steps_type>>::return_type>;
            std::optional<return_type> tmp = std::get<idx>(steps)(std::get<argument_type>(data));
            if (tmp.has_value()) {
                data = tmp.value();
                return idx + 1;
            }
            return idx;
        };
    }

    // We have to iterate over tuple because there can be functions with same signature but with
    // different logic, or even same function can be repeated. So std::get by type may not help us.
    template <size_t... Idx>
    static constexpr auto invoke_dispatch_table(std::index_sequence<Idx...>) {
        std::array<uint8_t(*)(steps_type&, current_arguments_type&), sizeof...(Idx)>
            invoke_dispatch = {make_invoker<Idx>()...};
        return invoke_dispatch;
    }

    // It can be implemented in terms of 'is_finished()' + 'advance()' but that would mean extra
    // function calls.
    inline bool execute_from(uint8_t begin_idx) {
        constexpr auto table =
            invoke_dispatch_table(std::make_index_sequence<sizeof...(Steps)>{});
        size_t previous = 0;
        for (uint8_t i = begin_idx; i < sizeof...(Steps); ++i) {
            previous = _current;
            _current = table[i](_steps, _current_args);
            if (_current == previous) {
                return false;
            }
        }
        return true;
    }

    bool execute_current() {
        constexpr auto table =
            invoke_dispatch_table(std::make_index_sequence<sizeof...(Steps)>{});
        size_t previous = _current;
        _current = table[_current](_steps, _current_args);
        return _current > previous;
    }

    // ----- Instantiate deserialization methods -----    

    inline void deserialize_arguments(std::string parameters) {
        if (_current < sizeof...(Steps)) {
            constexpr auto table =
                marshalling::deserialize_dispatch_table(std::make_index_sequence<sizeof...(Steps)>{});
            table[_current](_current_args, std::move(parameters));
        }
        else {
            _current_args = result_type{std::move(parameters)};
        }
    }

    // ----- Instantiate serialization methods -----

    inline std::string serialize_current_args() const {
        if (_current < sizeof...(Steps)) {
            constexpr auto table =
                marshalling::serialize_dispatch_table(std::make_index_sequence<sizeof...(Steps)>{});
            return table[_current](_current_args);
        }
        else {
            return std::get<result_type>(_current_args).serialize();
        }
    }

    // ----- Data members and aliases -----

    using steps_type = std::tuple<Steps...>;
    static_assert(
        std::is_default_constructible_v<
            std::decay_t<typename signature<std::tuple_element_t<0, steps_type>>::arg_type>>,
        "The argument type of the first step must be default-constructible"
    );
    using result_type = std::decay_t<
        typename signature<std::tuple_element_t<sizeof...(Steps) - 1, steps_type>>::return_type>;
    using current_arguments_type =
        unique_variant<std::decay_t<typename signature<Steps>::arg_type>..., result_type>;
    using marshalling = MarshallingInvokeTables<steps_type, current_arguments_type>;

    using all_serializable =
        typename std::conditional<
            is_serializable<result_type>::value &&
                (is_serializable<std::decay_t<typename signature<Steps>::arg_type>>::value && ...),
            std::true_type,
            std::false_type>::type;
    static_assert(all_serializable::value,
                  "All arguments and return type of the last step must be (de-)serializable.");

    steps_type _steps;
    current_arguments_type _current_args;
    uint8_t _current;
};

}; // namespace steps_chain
