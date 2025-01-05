#pragma once
#include "bort/Frontend/Symbol.hpp"
#include "bort/IR/Instruction.hpp"
#include "bort/IR/VariableUse.hpp"
#include <utility>

namespace bort::ir {

class AllocaInst final : public Instruction {
public:
  AllocaInst(const Ref<Variable>& variable, ValueRef elementSize,
             ValueRef numElements)
      : Instruction{ 2, VariableUse::createUnique(variable) } {
    m_Operands[s_ElementSizeIdx] = std::move(elementSize);
    m_Operands[s_NumElementsIdx] = std::move(numElements);
  }

  [[nodiscard]] auto getElementSize() const -> ValueRef {
    return getOperand(s_ElementSizeIdx);
  }
  [[nodiscard]] auto getNumElements() const -> ValueRef {
    return getOperand(s_NumElementsIdx);
  }

private:
  static constexpr size_t s_ElementSizeIdx{ 1 };
  static constexpr size_t s_NumElementsIdx{ 2 };
};

} // namespace bort::ir
