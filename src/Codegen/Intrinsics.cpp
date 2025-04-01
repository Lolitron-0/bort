#include "bort/Codegen/Instinsics.hpp"
#include <fmt/color.h>
#include <fmt/format.h>

namespace bort::codegen::rv::intrinsics {

auto getMacroName(MacroID id) -> std::string {
  switch (id) {
  case MacroID::ElemPtr:
    return "intr.elem.ptr";
  }
}

/// general idea of all macros: very inefficient, but better place
/// additional store/load here, than to allocate another register for
/// index
auto getDefinition(MacroID id) -> std::string {
  switch (id) {
  case MacroID::ElemPtr:
    return fmt::format(R"(
.macro {} %ptr %index %stride_shift
    addi sp, sp, -4
    sw %index, 0(sp)
    slli %index, %index, %stride_shift
    add %ptr, %ptr, %index
    lw %index, 0(sp)
    addi sp, sp 4
.end_macro)",
                       getMacroName(id));
  }
}

} // namespace bort::codegen::rv::intrinsics
