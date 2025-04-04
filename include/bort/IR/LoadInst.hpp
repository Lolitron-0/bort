#pragma once
#include "bort/Basic/Assert.hpp"
#include "bort/IR/Constant.hpp"
#include "bort/IR/Instruction.hpp"
#include "bort/IR/Value.hpp"
#include <utility>

namespace bort::ir {
class LoadInst final : public ir::Instruction {
public:
  LoadInst(ValueRef destination, ir::ValueRef loc, Ref<IntegralConstant> bytes)
      : Instruction{ 2, std::move(destination) } {
    setLoc(std::move(loc));
    setOperand(s_BytesIdx, std::move(bytes));
  }

  void setLoc(ir::ValueRef loc) {
    setOperand(s_LocIdx, std::move(loc));
  }

  [[nodiscard]] auto getLoc() const -> ir::ValueRef {
    return getOperand(s_LocIdx);
  }

  [[nodiscard]] auto getBytes() const -> Ref<IntegralConstant> {
    bort_assert(isaRef<IntegralConstant>(getOperand(s_BytesIdx)),
                "Store size should be constant");
    return dynCastRef<IntegralConstant>(getOperand(s_BytesIdx));
  }

private:
  static constexpr size_t s_LocIdx{ 1 };
  static constexpr size_t s_BytesIdx{ 2 }; ///< 1 - lb, 4 - lw, ...
};
} // namespace bort::ir
