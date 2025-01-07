#pragma once
#include "bort/Basic/Assert.hpp"
#include "bort/IR/BasicBlock.hpp"
#include "bort/IR/Instruction.hpp"

namespace bort::ir {

class BranchInst final : public Instruction {
public:
  explicit BranchInst(ValueRef condition = nullptr)
      : Instruction((condition ? 1 : 0)) {
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

  [[nodiscard]] auto isConditional() const -> bool {
    return getNumOperands() > 0;
  }

private:
  static constexpr size_t s_ConditionIdx{ 0 };

  const BasicBlock* m_Target{ nullptr };
};

} // namespace bort::ir
