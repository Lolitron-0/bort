#pragma once
#include <utility>

#include "bort/Frontend/Type.hpp"
#include "bort/IR/Instruction.hpp"

namespace bort::ir {

class AllocaInst : public Instruction {
public:
  explicit AllocaInst(TypeRef type, std::string name,
                      ValueRef elementSize, ValueRef numElements)
      : Instruction{ Value::createUnique(std::move(type),
                                         std::move(name)) },
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
