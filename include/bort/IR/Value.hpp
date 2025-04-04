#pragma once
#include "bort/Basic/Ref.hpp"
#include "bort/Frontend/Type.hpp"
#include "bort/IR/Metadata.hpp"
#include <optional>

namespace bort::ir {

class Value;
class ValueLoc;

using ValueRef = Ref<Value>;

class Value {
public:
  Value(const Value& other)
      : m_Type{ other.m_Type },
        m_Name{ other.m_Name },
        m_MDList{ makeUnique<MDList>(*other.m_MDList) } {
  }
  Value(Value&& other) = default;

  auto operator=(Value other) -> Value& {
    std::swap(m_Type, other.m_Type);
    std::swap(m_Name, other.m_Name);
    std::swap(m_MDList, other.m_MDList);
    return *this;
  }

  virtual ~Value() = default;

  // static auto get(std::string name) -> ValueRef;

  [[nodiscard]] auto getType() const -> TypeRef {
    return m_Type;
  }

  [[nodiscard]] auto getName() const -> std::string {
    return m_Name;
  }

  template <MetadataClass T>
  void addMDNode(T data) {
    m_MDList->add(std::move(data));
  }

  template <MetadataClass T>
  auto getMDNode() -> T* {
    return m_MDList->get<T>();
  }

  template <MetadataClass T>
  auto getMDNode() const -> const T* {
    return m_MDList->get<T>();
  }

  [[nodiscard]] auto getMDRange() const {
    return m_MDList->nodes();
  }

  static auto formatValue(const ValueRef& value)
      -> std::optional<std::string>;

protected:
  explicit Value(TypeRef type, std::string name = "")
      : m_Type{ std::move(type) },
        m_Name{ std::move(name) },
        m_MDList{ makeUnique<MDList>() } {
  }

private:
  TypeRef m_Type;
  std::string m_Name;
  Unique<MDList> m_MDList;
};

class Operand : public Value {
protected:
  using Value::Value;
};

} // namespace bort::ir
