#pragma once
#include "bort/Basic/Assert.hpp"
#include "bort/Frontend/Type.hpp"
#include "bort/IR/Instruction.hpp"
#include "bort/IR/Value.hpp"

namespace bort::ir {

class GepInst final : public Instruction {
public:
  GepInst(ValueRef destination, ValueRef basePtr, ValueRef index)
      : Instruction{ 2, std::move(destination) } {
    bort_assert(isaRef<PointerType>(basePtr->getType()),
                "GepInst got non pointer type");
    setArray(std::move(basePtr));
    setIndex(std::move(index));
  }

  [[nodiscard]] auto getBasePtr() const -> ValueRef {
    return getOperand(BaseIdx);
  }
  [[nodiscard]] auto getIndex() const -> ValueRef {
    return getOperand(IndexIdx);
  }

  void setArray(ValueRef value) {
    setOperand(BaseIdx, std::move(value));
  }
  void setIndex(ValueRef value) {
    setOperand(IndexIdx, std::move(value));
  }

  static constexpr size_t BaseIdx{ 1 };
  static constexpr size_t IndexIdx{ 2 };
};

} // namespace bort::ir
