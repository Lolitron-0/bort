#pragma once
#include "bort/IR/Value.hpp"
#include <vector>

namespace bort::ir {

class Instruction : public Value {
public:
  [[nodiscard]] auto getDestination() const -> ValueRef;

  [[nodiscard]] auto getOperand(size_t index) const -> ValueRef;
  [[nodiscard]] auto getNumOperands() const -> size_t {
    return m_NumOperands;
  }

protected:
  explicit Instruction(size_t numArgs, ValueRef destination);
  explicit Instruction(size_t numArgs);

  std::vector<ValueRef> m_Operands;

private:
  static constexpr size_t s_DestinationIdx{ 0 };

  size_t m_NumOperands;
};

} // namespace bort::ir
