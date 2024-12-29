#pragma once
#include "bort/Basic/Ref.hpp"
#include "bort/Frontend/Type.hpp"
#include <unordered_map>

namespace bort::ir {

class Value;

using ValueRef = Ref<Value>;

class Value {
public:
  virtual ~Value() = default;


  static auto get(std::string name) -> ValueRef;

  [[nodiscard]] auto getType() const -> const TypeRef& {
    return m_Type;
  }

  [[nodiscard]] auto getName() const -> const std::string& {
    return m_Name;
  }

protected:
  explicit Value(TypeRef type, std::string name = "")
      : m_Type{ std::move(type) },
        m_Name{ std::move(name) } {
  }

private:
  TypeRef m_Type;
  std::string m_Name;
};

} // namespace bort::ir
