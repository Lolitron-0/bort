#pragma once
#include "bort/IR/Instruction.hpp"
#include "bort/IR/Value.hpp"
#include <utility>

namespace bort::ir {

class RetInst : public Instruction {
public:
  RetInst()
      : Instruction(0) {
  }
  explicit RetInst(ValueRef returnValue)
      : Instruction{ returnValue ? 1U : 0U } {
    setOperand(s_ReturnValueIndex, std::move(returnValue));
  }

  [[nodiscard]] auto hasValue() const -> bool {
    return getOperand(s_ReturnValueIndex) != nullptr;
  }

  [[nodiscard]] auto getValue() const -> ValueRef {
    return getOperand(s_ReturnValueIndex);
  }

  void setValue(ValueRef value) {
    setOperand(s_ReturnValueIndex, std::move(value));
  }

private:
  static constexpr size_t s_ReturnValueIndex{ 0 };
};

} // namespace bort::ir
