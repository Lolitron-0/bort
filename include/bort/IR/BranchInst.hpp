#pragma once
#include "bort/Basic/Assert.hpp"
#include "bort/IR/BasicBlock.hpp"
#include "bort/IR/Instruction.hpp"
#include "bort/Lex/Token.hpp"
#include <optional>

namespace bort::ir {

class BranchInst final : public Instruction {
public:
  BranchInst()
      : Instruction(0) {
  }

  BranchInst(ValueRef lhs, ValueRef rhs, TokenKind mode)
      : Instruction((lhs && rhs ? 2 : 0)),
        m_Mode{ mode } {
    if (lhs && rhs) {
      setOperands(std::move(lhs), std::move(rhs));
    }
  }

  void setTarget(const BasicBlock* block) {
    m_Target = block;
  }
  void setOperands(ValueRef lhs, ValueRef rhs) {
    m_Operands[LHSIdx] = std::move(lhs);
    m_Operands[RHSIdx] = std::move(rhs);
  }

  [[nodiscard]] auto getTarget() const -> const BasicBlock* {
    bort_assert(m_Target, "Target not set");
    return m_Target;
  }

  [[nodiscard]] auto getOperands() const
      -> std::pair<ValueRef, ValueRef> {
    bort_assert(isConditional(), "getCondition on unconditional branch");
    return { getOperand(LHSIdx), getOperand(RHSIdx) };
  }

  [[nodiscard]] auto getLHS() const -> ValueRef {
    bort_assert(isConditional(), "getLHS on unconditional branch");
    return getOperand(LHSIdx);
  }

  [[nodiscard]] auto getRHS() const -> ValueRef {
    bort_assert(isConditional(), "getRHS on unconditional branch");
    return getOperand(RHSIdx);
  }

  [[nodiscard]] auto getMode() const -> TokenKind {
    bort_assert(m_Mode, "Mode not set");
    return *m_Mode;
  }

  [[nodiscard]] auto isConditional() const -> bool {
    return getNumOperands() > 0;
  }

  static constexpr size_t LHSIdx{ 0 };
  static constexpr size_t RHSIdx{ 1 };

private:
  const BasicBlock* m_Target{ nullptr };
  std::optional<TokenKind> m_Mode{ std::nullopt };
};

} // namespace bort::ir
