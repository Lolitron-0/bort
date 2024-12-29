#pragma once
#include "bort/IR/Instruction.hpp"
#include <utility>

namespace bort::ir {

class MoveInst : public Instruction {
public:
  explicit MoveInst(ValueRef dst, ValueRef src)
      : Instruction{ std::move(dst) },
        m_Src{ std::move(src) } {
  }

  [[nodiscard]] auto getSrc() const -> const ValueRef& {
    return m_Src;
  }

private:
  ValueRef m_Src;
};

} // namespace bort::ir
