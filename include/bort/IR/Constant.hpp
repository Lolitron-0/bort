#pragma once
#include "bort/IR/Value.hpp"

namespace bort::ir {

struct Constant : public Value {
protected:
  using Value::Value;
};

class IntConstant final : public Constant {
public:
  /// @todo getOrCreate
  static auto create(int32_t value) -> Unique<IntConstant> {
    return Unique<IntConstant>(new IntConstant{ value });
  }

  [[nodiscard]] auto getValue() const -> int {
    return m_Value;
  }

private:
  explicit IntConstant(int32_t value)
      : Constant{ IntType::get() },
        m_Value{ value } {
  }

  int32_t m_Value;
};

} // namespace bort::ir
