#pragma once

#include "util.h"

#include <array>
#include <tuple>

namespace steps_chain {
namespace helpers {

template<typename Steps, typename Arg>
struct MarshallingInvokeTables {
    template <size_t idx>
    static constexpr auto make_deserializer() {
        return [](Arg& data, std::string parameters) -> void {
            using argument_type = std::decay_t<
                typename signature<std::tuple_element_t<idx, Steps>>::arg_type>;
            data = argument_type{std::move(parameters)};
        };
    }

    template <size_t... Idx>
    static constexpr auto deserialize_dispatch_table(std::index_sequence<Idx...>) {
        std::array<void(*)(Arg&, std::string), sizeof...(Idx)>
            deserialize_dispatch = {make_deserializer<Idx>()...};
        return deserialize_dispatch;
    }

    template <size_t idx>
    static constexpr auto make_serializer() {
        return [](const Arg& data) -> std::string {
            using arg_type = std::decay_t<
                typename signature<std::tuple_element_t<idx, Steps>>::arg_type>;
            return std::get<arg_type>(data).serialize();
        };
    }

    template <size_t... Idx>
    static constexpr auto serialize_dispatch_table(std::index_sequence<Idx...>) {
        std::array<std::string(*)(const Arg&), sizeof...(Idx)>
            serialize_dispatch = {make_serializer<Idx>()...};
        return serialize_dispatch;
    }
};

}; // namespace helpers
}; // namespace steps_chain
