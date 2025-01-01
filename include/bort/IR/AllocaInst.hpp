#pragma once
#include "bort/Frontend/Symbol.hpp"
#include "bort/Frontend/Type.hpp"
#include "bort/IR/Instruction.hpp"
#include "bort/IR/VariableUse.hpp"
#include <utility>

namespace bort::ir {

class AllocaInst : public Instruction {
public:
  explicit AllocaInst(Ref<Variable> variable, ValueRef elementSize,
                      ValueRef numElements)
      : Instruction{ VariableUse::createUnique(std::move(variable)) },
        m_ElementSize{ std::move(elementSize) },
        m_NumElements{ std::move(numElements) } {
  }

  [[nodiscard]] auto getElementSize() const -> const ValueRef& {
    return m_ElementSize;
  }
  [[nodiscard]] auto getNumElements() const -> const ValueRef& {
    return m_NumElements;
  }

private:
  ValueRef m_ElementSize;
  ValueRef m_NumElements;
};

} // namespace bort::ir
