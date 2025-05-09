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
  Value(const Value& other);
  Value(Value&& other) = default;

  auto operator=(Value other) -> Value&;

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
  void removeMDNode() {
    m_MDList->remove<T>();
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
  explicit Value(TypeRef type, std::string name = "");

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
