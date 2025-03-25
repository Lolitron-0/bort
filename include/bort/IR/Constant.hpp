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
  static auto getOrCreate(int32_t value) -> Ref<IntConstant> {
    static std::unordered_map<int32_t, Ref<IntConstant>> s_Registry{};
    if (!s_Registry.contains(value)) {
      s_Registry[value] = Ref<IntConstant>(new IntConstant{ value });
    }
    return s_Registry.at(value);
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
