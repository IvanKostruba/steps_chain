#pragma once

#include <string>
#include <type_traits>
#include <variant>

namespace steps_chain {
namespace helpers {

//---------- SFINAE detectors for function return and argument types ----------

// Dispatch lambda types to correct specialization
template <typename T>
struct signature {
    using return_type = typename signature<decltype(&T::operator())>::return_type;
    using arg_type = typename signature<decltype(&T::operator())>::arg_type;
    using context_type = typename signature<decltype(&T::operator())>::context_type;;
};

// Function type unwrapper for two arguments
template <typename R, typename T, typename C>
struct signature<R(T, C)> {
    using return_type = R;
    using arg_type = T;
    using context_type = C;
};

// Core function type unwrapper
template <typename R, typename T>
struct signature<R(T)> {
    using return_type = R;
    using arg_type = T;
    using context_type = void;
};

template <typename R>
struct signature<R()> {
    using return_type = R;
    using arg_type = void;
    using context_type = void;
};

// We are not interested in functions with 3+ arguments
template <typename R, typename T, typename... Ts>
struct signature<R(T, Ts...)> {
    using return_type = void;
    using arg_type = void;
    using context_type = void;
};

// Match with function pointers
template <typename R, typename T>
struct signature<R (*)(T)> : public signature<R(T)> {};

template <typename R>
struct signature<R (*)()> : public signature<R()> {};

template <typename R, typename T, typename... Ts>
struct signature<R (*)(T, Ts...)> : public signature<R(T, Ts...)> {};

// Match with functors and lambdas and class method pointers
template <typename R, typename C, typename T>
struct signature<R (C::*)(T) const> : public signature<R(T)> {};

template <typename R, typename C>
struct signature<R (C::*)() const> : public signature<R()> {};

template <typename R, typename C, typename T, typename... Ts>
struct signature<R (C::*)(T, Ts...) const> : public signature<R(T, Ts...)> {};

//---------- SFINAE parameters type requirements ----------

template <class P, class = void>
struct is_serializable : std::false_type {};

template <class P>
struct is_serializable<P, std::void_t<
    std::enable_if_t<
        std::is_same_v<
            decltype(std::declval<P>().serialize()),
            std::string>
        && std::is_constructible<P, std::string>::value,
        int>,
    void>> : std::true_type {};

//---------- SFINAE check if functions are eligible to chaining in given order ----------

template <typename T, typename... Ts>
constexpr bool are_chainable() {
    using return_type = std::decay_t<typename signature<T>::return_type>;
    using arg_types = std::tuple<std::decay_t<typename signature<Ts>::arg_type>...>;
    using context_type = typename signature<T>::context_type;
    using next_contexts = std::tuple<typename signature<Ts>::context_type...>;
    if constexpr (sizeof...(Ts) > 0) {
        return std::is_same_v<
                    return_type, std::tuple_element_t<0, arg_types>> &&
               std:: is_same_v<
                    context_type, std::tuple_element_t<0, next_contexts>> &&
               are_chainable<Ts...>();
    }
    else {
        return true;
    }
}

//---------- Extract unique types from parameter pack into variant ----------

template <class T>
struct type_identity {
    using type = T;
};

template <typename T, typename... Ts>
struct unique : type_identity<T> {};

template <typename... Ts, typename U, typename... Us>
struct unique<std::variant<Ts...>, U, Us...>
    : std::conditional_t<(std::is_same_v<U, Ts> || ...),
        unique<std::variant<Ts...>, Us...>,
        unique<std::variant<Ts..., U>, Us...>
    > {};

template <typename... Ts>
using unique_variant = typename unique<std::variant<>, Ts...>::type;

}; // namespace helpers
}; // namespace steps_chain
