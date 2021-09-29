#pragma once

#include "util.h"
#include "marshalling_helper.h"

#include <array>
#include <tuple>

namespace steps_chain {

using namespace helpers;

// The idea of this class is fundamentally the same as StepsChain. The difference is that callables
// here require some external context to run, not just result of the previous call, so they take
// two arguments, second being the context. The context argument type must be identical for all the
// callables (i.e. always passed by const ref or always by value). If context is passed by value
// it's still being moved 3 times internally.

template <typename... Steps>
class ContextStepsChain
{
public:
    using steps_type = std::tuple<Steps...>;
    using context_type = typename signature<std::tuple_element_t<0, steps_type>>::context_type;
    static_assert(!std::is_same_v<context_type, void>,
                  "ContextStepsChain steps must have 2 arguments.");
    static_assert(
        std::is_default_constructible_v<
            std::decay_t<typename signature<std::tuple_element_t<0, steps_type>>::arg_type>>,
        "The argument type of the first step must be default-constructible"
    );
    static_assert(
        !std::is_reference_v<context_type> ||
        std::is_const_v<std::remove_reference_t<context_type>>,
        "Context must be passed by value or by const reference.");
    static_assert(are_chainable<Steps...>(),
                  "Return type of the previous function must be the same as argument type of the " \
                  "next, and second argument ('context') types must be identical." \
                  "If you use optional return type, next function argument should not be optional");

    ContextStepsChain(Steps... steps) : _steps{steps...}, _current{0} {}

    // Run all remaining steps, beginnig with given index.
    bool run(std::string parameters, context_type ctx, uint8_t begin_idx = 0) {
        if (begin_idx >= sizeof...(Steps)) {
            return false;
        }
        initialize(std::move(parameters), begin_idx);
        return execute_from(begin_idx, std::move(ctx));
    }

    // Just initializer, intended to be used in pair with advance()
    bool initialize(std::string parameters, uint8_t current_idx = 0) {
        if (current_idx >= sizeof...(Steps)) {
            return false;
        }
        deserialize_arguments(current_idx, std::move(parameters));
        _current = current_idx;
        return true;
    }

    bool advance(context_type ctx) {
        if (_current >= sizeof...(Steps)) {
            return false;
        }
        return execute_current(std::move(ctx));
    }

    // Run all remaining steps, beginning with current index.
    bool resume(context_type ctx) {
        if (_current >= sizeof...(Steps)) {
            return false;
        }
        return execute_from(_current, std::move(ctx));
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
        return [](steps_type& steps, current_arguments_type& data, context_type ctx) -> uint8_t {
            using argument_type = std::decay_t<
                typename signature<std::tuple_element_t<idx, steps_type>>::arg_type>;
            using return_type = std::decay_t<
                typename signature<std::tuple_element_t<idx, steps_type>>::return_type>;
            std::optional<return_type> tmp = 
                std::get<idx>(steps)(std::get<argument_type>(data), std::move(ctx));
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
        std::array<
            uint8_t(*)(
                steps_type&,
                current_arguments_type&,
                context_type
            ), sizeof...(Idx)> invoke_dispatch = {make_invoker<Idx>()...};
        return invoke_dispatch;
    }

    bool execute_from(uint8_t begin_idx, context_type ctx) {
        constexpr auto table =
            invoke_dispatch_table(std::make_index_sequence<sizeof...(Steps)>{});
        size_t previous = 0;
        for (uint8_t i = begin_idx; i < sizeof...(Steps); ++i) {
            previous = _current;
            _current = table[i](_steps, _current_args, ctx);
            if (_current == previous) {
                return false;
            }
        }
        return true;
    }

    bool execute_current(context_type ctx) {
        constexpr auto table =
            invoke_dispatch_table(std::make_index_sequence<sizeof...(Steps)>{});
        size_t previous = _current;
        _current = table[_current](_steps, _current_args, std::move(ctx));
        return _current > previous;
    }

    // ----- Instantiate deserialization methods -----    

    inline void deserialize_arguments(uint8_t step, std::string parameters) {
        constexpr auto table =
            marshalling::deserialize_dispatch_table(std::make_index_sequence<sizeof...(Steps)>{});
        table[step](_current_args, std::move(parameters));
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
