#pragma once
#include "bort/Basic/Assert.hpp"
#include "bort/Frontend/Type.hpp"
#include "bort/IR/Value.hpp"

namespace bort::ir {

struct Constant : public Value {
protected:
  using Value::Value;
};

class IntegralConstant final : public Constant {
public:
  static auto getOrCreate(int32_t value,
                          const TypeRef& type) -> Ref<IntegralConstant> {
    if (type == IntType::get()) {
      return getInt(value);
    }
    if (type == CharType::get()) {
      return getChar(static_cast<char>(value));
    }
    bort_assert(false, "Wrong type for integral constant");
    return nullptr;
  }

  static auto getInt(int32_t value) -> Ref<IntegralConstant> {
    static std::unordered_map<int32_t, Ref<IntegralConstant>>
        s_Registry{};
    if (!s_Registry.contains(value)) {
      s_Registry[value] = Ref<IntegralConstant>(
          new IntegralConstant{ value, IntType::get() });
    }
    return s_Registry.at(value);
  }

  static auto getChar(char value) -> Ref<IntegralConstant> {
    static std::unordered_map<char, Ref<IntegralConstant>> s_Registry{};
    if (!s_Registry.contains(value)) {
      s_Registry[value] = Ref<IntegralConstant>(
          new IntegralConstant{ value, CharType::get() });
    }
    return s_Registry.at(value);
  }

  [[nodiscard]] auto getValue() const -> int {
    return m_Value;
  }

private:
  explicit IntegralConstant(int32_t value, TypeRef type)
      : Constant{ std::move(type) },
        m_Value{ value } {
  }

  int32_t m_Value;
};

} // namespace bort::ir
