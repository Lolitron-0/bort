#pragma once
#include "bort/Basic/Assert.hpp"
#include "bort/IR/Constant.hpp"
#include "bort/IR/Instruction.hpp"
#include "bort/IR/Value.hpp"
#include <utility>

namespace bort::ir {
class StoreInst final : public ir::Instruction {
public:
  StoreInst(ValueRef source, ir::ValueRef loc, Ref<IntegralConstant> bytes)
      : Instruction{ 3 } {
    m_Operands[s_SrcIdx] = std::move(source);
    setLoc(std::move(loc));
    setOperand(s_BytesIdx, std::move(bytes));
  }

  void setLoc(ir::ValueRef loc) {
    setOperand(s_LocIdx, std::move(loc));
  }

  void setSource(ValueRef source) {
    setOperand(s_SrcIdx, std::move(source));
  }

  [[nodiscard]] auto getLoc() const -> ir::ValueRef {
    return getOperand(s_LocIdx);
  }

  [[nodiscard]] auto getSource() const -> ValueRef {
    return getOperand(s_SrcIdx);
  }

  [[nodiscard]] auto getBytes() const -> Ref<IntegralConstant> {
    bort_assert(isaRef<IntegralConstant>(getOperand(s_BytesIdx)),
                "Store size should be constant");
    return dynCastRef<IntegralConstant>(getOperand(s_BytesIdx));
  }

private:
  static constexpr size_t s_SrcIdx{ 0 };
  static constexpr size_t s_LocIdx{ 1 };
  static constexpr size_t s_BytesIdx{ 2 }; ///< 1 - lb, 4 - lw, ...
};
} // namespace bort::ir
