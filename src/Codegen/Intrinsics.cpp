#include "bort/Codegen/Intrinsics.hpp"
#include <fmt/color.h>
#include <fmt/format.h>

namespace bort::codegen::rv::intrinsics {

auto getMacroName(MacroID id) -> std::string {
  switch (id) {
  case MacroID::ElemPtr:
    return "i.elem.ptr";
  }
}

/// general idea of all macros: they can be very inefficient, but it's
/// better to place additional store/load here, than to allocate another
/// register in codegen
auto getDefinition(MacroID id) -> std::string {
  switch (id) {
  case MacroID::ElemPtr:
    return fmt::format(R"(
.macro {} %ptr_reg %index_reg %stride_shift_imm
    addi sp, sp, -4
    sw %index_reg, 0(sp)
    slli %index_reg, %index_reg, %stride_shift_imm
    add %ptr_reg, %ptr_reg, %index_reg
    lw %index_reg, 0(sp)
    addi sp, sp 4
.end_macro)",
                       getMacroName(id));
  }
}

} // namespace bort::codegen::rv::intrinsics
