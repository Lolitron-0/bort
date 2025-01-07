#pragma once
#include "bort/Basic/Casts.hpp"
#include "bort/Codegen/MachineRegister.hpp"
#include "bort/Codegen/ValueLoc.hpp"
#include "bort/IR/Instruction.hpp"

namespace bort::codegen {
class StoreInst final : public ir::Instruction {
public:
  StoreInst(Ref<MachineRegister> source, Ref<ValueLoc> loc, size_t bytes)
      : Instruction{ 1 },
        m_Loc{ std::move(loc) },
        m_Bytes{ bytes } {
    m_Operands[s_SrcIdx] = std::move(source);
  }

  [[nodiscard]] auto getLoc() const -> Ref<ValueLoc> {
    return m_Loc;
  }

  [[nodiscard]] auto getSource() const -> Ref<MachineRegister> {
    return dynCastRef<MachineRegister>(getOperand(s_SrcIdx));
  }

  [[nodiscard]] auto getBytes() const -> size_t {
    return m_Bytes;
  }

private:
  static constexpr size_t s_SrcIdx{ 0 };

  Ref<ValueLoc> m_Loc;
  size_t m_Bytes; ///< 1 - lb, 4 - lw, ...
};
} // namespace bort::codegen
