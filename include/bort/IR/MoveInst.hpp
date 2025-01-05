#pragma once
#include "bort/IR/Instruction.hpp"
#include <utility>

namespace bort::ir {

class MoveInst final : public Instruction {
public:
  explicit MoveInst(ValueRef dst, ValueRef src)
      : Instruction{ 1, std::move(dst) } {
    m_Operands[s_SrcIdx] = std::move(src);
  }

  [[nodiscard]] auto getSrc() const -> const ValueRef& {
    return m_Operands[s_SrcIdx];
  }

private:
  static constexpr size_t s_SrcIdx{ 1 };
};

} // namespace bort::ir
