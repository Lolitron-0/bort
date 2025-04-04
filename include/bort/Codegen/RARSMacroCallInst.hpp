#pragma once
#include "bort/Basic/Assert.hpp"
#include "bort/Codegen/Intrinsics.hpp"
#include "bort/IR/Instruction.hpp"
#include "bort/IR/Value.hpp"

namespace bort::codegen::rv {

class RARSMacroCallInst final : public ir::Instruction {
public:
  template <size_t N>
  RARSMacroCallInst(intrinsics::MacroID macro,
                    std::array<ir::ValueRef, N> args)
      : Instruction{ args.size() },
        m_NumArgs{ args.size() },
        m_MacroID{ macro } {
    bort_assert(intrinsics::checkSignature(macro, args),
                "Macro with invalid signature instantiated");
    for (size_t i{ 0 }; i < args.size(); i++) {
      setArg(i, args[i]);
    }
  }

  [[nodiscard]] auto getArg(size_t index) const -> ir::ValueRef {
    return m_Operands[index];
  }

  [[nodiscard]] auto getArgs() const -> const std::vector<ir::ValueRef>& {
    return m_Operands;
  }

  [[nodiscard]] auto getNumArgs() const -> size_t {
    return m_NumArgs;
  }

  [[nodiscard]] auto getMacroID() const -> intrinsics::MacroID {
    return m_MacroID;
  }

  void setArg(size_t index, ir::ValueRef value) {
    m_Operands[index] = std::move(value);
  }

private:
  size_t m_NumArgs;
  intrinsics::MacroID m_MacroID;
};

} // namespace bort::codegen::rv
