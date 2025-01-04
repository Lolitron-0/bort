#pragma once
#include "bort/Basic/Assert.hpp"
#include "bort/Frontend/Type.hpp"
#include "bort/IR/BasicBlock.hpp"
#include "bort/IR/Instruction.hpp"

namespace bort::ir {

class BranchInst final : public Instruction {
public:
  explicit BranchInst(ValueRef condition = nullptr)
      : Instruction{ VoidType::get() },
        m_Condition{ std::move(condition) } {
  }

  void setTarget(const BasicBlock* block) {
    m_Target = block;
  }

  [[nodiscard]] auto getTarget() const -> const BasicBlock* {
    bort_assert(m_Target, "Target not set");
    return m_Target;
  }

  [[nodiscard]] auto getCondition() const -> const ValueRef& {
    bort_assert(isConditional(), "getCondition on unconditional branch");
    return m_Condition;
  }

  [[nodiscard]] auto isConditional() const -> bool {
    return m_Condition != nullptr;
  }

private:
  const BasicBlock* m_Target{ nullptr };
  ValueRef m_Condition;
};

} // namespace bort::ir
