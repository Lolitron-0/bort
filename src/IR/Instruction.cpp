#include "bort/IR/Instruction.hpp"
#include "bort/Basic/Assert.hpp"
#include "bort/Frontend/Type.hpp"
#include <fmt/format.h>

namespace bort::ir {

auto Instruction::getDestination() const -> ValueRef {
  bort_assert(getType() != VoidType::get(),
              "getDestination on void instruction");
  return m_Operands[s_DestinationIdx];
}

auto Instruction::getOperand(size_t index) const -> ValueRef {
  bort_assert(index < m_NumOperands, "Index out of range");
  return m_Operands[index];
}

Instruction::Instruction(size_t numArgs, ValueRef destination)
    : Value{ destination->getType() },
      m_Operands(numArgs + 1),
      m_NumOperands{ numArgs + 1 } {
  m_Operands[s_DestinationIdx] = std::move(destination);
}
Instruction::Instruction(size_t numArgs)
    : Value{ VoidType::get() },
      m_Operands(numArgs),
      m_NumOperands{ numArgs } {
}

auto Instruction::getNumOperands() const -> size_t {
  return m_NumOperands;
}

void Instruction::setDestination(ValueRef value) {
  m_Operands[s_DestinationIdx] = std::move(value);
}
void Instruction::setOperand(size_t index, ValueRef value) {
  bort_assert(index < m_NumOperands, "Index out of range");
  m_Operands[index] = std::move(value);
}
} // namespace bort::ir
