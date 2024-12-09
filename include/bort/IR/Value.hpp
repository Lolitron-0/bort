#pragma once
#include "bort/Basic/Ref.hpp"
#include "bort/Frontend/Type.hpp"

namespace bort::ir {

class Value {
public:
  virtual ~Value() = default;

  static auto create(TypeRef type,
                     std::string name = "") -> Unique<Value> {
    return Unique<Value>(new Value{ std::move(type), std::move(name) });
  };

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

using ValueRef = Ref<Value>;

} // namespace bort::ir
