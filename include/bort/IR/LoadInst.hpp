#pragma once
#include "bort/IR/Instruction.hpp"
#include "bort/IR/ValueLoc.hpp"

namespace bort::ir {
class LoadInst final : public Instruction {
public:
  LoadInst(ValueRef destination, Ref<ValueLoc> loc, size_t bytes)
      : Instruction{ 0, std::move(destination) },
        m_Loc{ std::move(loc) },
        m_Bytes{ bytes } {
  }

  [[nodiscard]] auto getLoc() const -> Ref<ValueLoc> {
    return m_Loc;
  }

  [[nodiscard]] auto getBytes() const -> size_t {
    return m_Bytes;
  }

private:
  Ref<ValueLoc> m_Loc;
  size_t m_Bytes; ///< 1 - lb, 4 - lw, ...
};
} // namespace bort::ir
