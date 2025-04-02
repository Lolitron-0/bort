#pragma once
#include "bort/Codegen/MachineRegister.hpp"
#include "bort/IR/Constant.hpp"
#include "bort/IR/Value.hpp"
#include <cstdint>
#include <cul/cul.hpp>
#include <string>
#include <tuple>
#include <utility>

namespace bort::codegen::rv::intrinsics {

enum class MacroID : uint8_t {
  ElemPtr
};

auto getMacroName(MacroID id) -> std::string;

auto getDefinition(MacroID id) -> std::string;

namespace detail {
template <typename... Ts>
auto checkRefTypes(std::array<ir::ValueRef, sizeof...(Ts)> params)
    -> bool {
  using Ts_ = std::tuple<Ts...>;
  return [&]<size_t... Is>(std::index_sequence<Is...>) {
    return ([&]<size_t i> {
      return isaRef<std::tuple_element_t<i, Ts_>>(params.at(i));
    }.template operator()<Is>() &&
            ...);
  }.template operator()(std::index_sequence_for<Ts...>());
}
} // namespace detail

/// i would really like to make a normal macro engine (such as constexpr
/// parsing of macro definitions), but...
template <size_t N>
auto checkSignature(MacroID id,
                    std::array<ir::ValueRef, N> params) -> bool {
  switch (id) {
  case MacroID::ElemPtr:
    return detail::checkRefTypes<MachineRegister, MachineRegister,
                                 ir::IntegralConstant>(params);
  }
}

} // namespace bort::codegen::rv::intrinsics
