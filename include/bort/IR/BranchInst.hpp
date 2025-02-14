#pragma once
#include "bort/Basic/Assert.hpp"
#include "bort/IR/BasicBlock.hpp"
#include "bort/IR/Instruction.hpp"

namespace bort::ir {

class BranchInst final : public Instruction {
public:
  explicit BranchInst(ValueRef condition = nullptr, bool negate = false)
      : Instruction((condition ? 1 : 0)),
        m_Negated{ negate } {
    if (condition) {
      m_Operands[s_ConditionIdx] = std::move(condition);
    }
  }

  void setTarget(const BasicBlock* block) {
    m_Target = block;
  }
  void setCondition(ValueRef condition) {
    m_Operands[s_ConditionIdx] = std::move(condition);
  }

  [[nodiscard]] auto getTarget() const -> const BasicBlock* {
    bort_assert(m_Target, "Target not set");
    return m_Target;
  }

  [[nodiscard]] auto getCondition() const -> ValueRef {
    bort_assert(isConditional(), "getCondition on unconditional branch");
    return getOperand(s_ConditionIdx);
  }

  [[nodiscard]] auto isNegated() const -> bool {
    bort_assert(isConditional(), "isNegated on unconditional branch");
    return m_Negated;
  }

  [[nodiscard]] auto isConditional() const -> bool {
    return getNumOperands() > 0;
  }

private:
  static constexpr size_t s_ConditionIdx{ 0 };

  const BasicBlock* m_Target{ nullptr };
  bool m_Negated{ false };
};

} // namespace bort::ir
