#pragma once
#include <cstdint>
#include <string>

namespace bort::codegen::rv::intrinsics {

enum class MacroID : uint8_t {
  ElemPtr
};

auto getDefinition(MacroID id) -> std::string;
auto getMacroName(MacroID id) -> std::string;

} // namespace bort::codegen::rv::intrinsics
