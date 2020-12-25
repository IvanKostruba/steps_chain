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
                  "Return type of the previous function must be the same as argument type of the next.");
    StepsChain(Steps... steps) : _steps{steps...}, _current{0} {}

    // Run all remaining steps, beginnig with given index.
    void run(std::string parameters, uint8_t begin_idx = 0) {
        if (begin_idx >= sizeof...(Steps)) {
            // TODO: introduce error handling, maybe exception?
            return;
        }
        initialize(std::move(parameters), begin_idx);
        execute_from(begin_idx);
    }

    // Just initializer, intended to be used in pair with advance()
    void initialize(std::string parameters, uint8_t current_idx = 0) {
        if (current_idx >= sizeof...(Steps)) {
            return;
        }
        deserialize_arguments(current_idx, std::move(parameters));
        _current = current_idx;
    }

    void advance() {
        if (_current >= sizeof...(Steps)) {
            return;
        }
        execute_current();
    }

    // Run all remaining steps, beginning with current index.
    void resume() {
        if (_current >= sizeof...(Steps)) {
            return;
        }
        execute_from(_current);
    }

    // Get step index and serialized arguments for current step so that they can be stored.
    // If final step was executed, final result will be returned.
    std::tuple<uint8_t, std::string> get_current_state() const {
        return std::make_tuple(_current, serialize_current_args());
    }

    bool is_finished() const { return _current == sizeof...(Steps); }

private:
    // ----- Iterate over callables and invoke them -----

    template <uint8_t idx>
    static auto make_invoker() {
        return [](steps_type& steps, current_arguments_type& data) -> uint8_t {
            using argument_type = std::decay_t<
                typename signature<std::tuple_element_t<idx, steps_type>>::arg_type>;
            data = std::get<idx>(steps)(std::get<argument_type>(data));
            return idx + 1;
        };
    }

    // We have to iterate over tuple because there can be functions with same signature but with
    // different logic, or even same function can be repeated. So std::get by type may not help us.
    template <size_t... Idx>
    static auto invoke_dispatch_table(std::index_sequence<Idx...>) {
        static std::array<uint8_t(*)(steps_type&, current_arguments_type&), sizeof...(Idx)>
            invoke_dispatch = {make_invoker<Idx>()...};
        return invoke_dispatch;
    }

    void execute_from(size_t begin_idx) {
        static const auto& table =
            invoke_dispatch_table(std::make_index_sequence<sizeof...(Steps)>{});
        for (uint8_t i = begin_idx; i < sizeof...(Steps); ++i) {
            _current = table[i](_steps, _current_args);
        }
    }

    void execute_current() {
        static const auto& table =
            invoke_dispatch_table(std::make_index_sequence<sizeof...(Steps)>{});
        _current = table[_current](_steps, _current_args);
    }

    // ----- Instantiate deserialization methods -----    

    void deserialize_arguments(uint8_t step, std::string parameters) {
        static const auto& table =
            marshalling::deserialize_dispatch_table(std::make_index_sequence<sizeof...(Steps)>{});
        table[step](_current_args, std::move(parameters));
    }

    // ----- Instantiate serialization methods -----

    std::string serialize_current_args() const {
        if (_current < sizeof...(Steps)) {
            static const auto& table =
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

    using all_de_serializable =
        typename std::conditional<
            is_serializable<result_type>::value &&
                (is_serializable<std::decay_t<typename signature<Steps>::arg_type>>::value && ...),
            std::true_type,
            std::false_type>::type;
    static_assert(all_de_serializable::value,
                  "All arguments and return type of the last step must be (de-)serializable.");

    steps_type _steps;
    current_arguments_type _current_args;
    uint8_t _current;
};

}; // namespace steps_chain
