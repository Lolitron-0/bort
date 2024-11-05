#pragma once
#include "bort/Basic/Assert.hpp"
#include "bort/Frontend/Type.hpp"
#include <initializer_list>
#include <string>
#include <vector>

namespace bort {

enum class ObjectKind {
  Variable,
  Function,
  NUM_OBJECT_KINDS
};

/// Some program entity representation
///
/// Symbol is a representation of any (i quess) named entitiy in a program
/// (function, variable, typedef, ...). The key concept in symbol
/// resolution is symbol shallowness: shallow symbols are just names by
/// which actual symbol information will be retrieved later (in resolution
/// process).
/// @todo typedef symbols
class Symbol {
public:
  Symbol(ObjectKind kind, std::string name, bool shallow)
      : m_Name{ std::move(name) },
        m_Kind{ kind },
        m_Shallow{ shallow } {
  }

  virtual ~Symbol() = default;

  [[nodiscard]] auto getName() const -> std::string {
    return m_Name;
  }
  [[nodiscard]] auto getKind() const -> ObjectKind {
    return m_Kind;
  }
  [[nodiscard]] auto isShallow() const -> bool {
    return m_Shallow;
  }

  void dump() const;
  [[nodiscard]] virtual auto toString() const -> std::string = 0;

protected:
  std::string m_Name;

private:
  ObjectKind m_Kind;
  bool m_Shallow;
};

class Variable final : public Symbol {
public:
  explicit Variable(std::string name)
      : Symbol{ ObjectKind::Variable, std::move(name), true },
        m_Type{ nullptr } {
  }
  Variable(std::string name, TypeRef type)
      : Symbol{ ObjectKind::Variable, std::move(name), false },
        m_Type{ std::move(type) } {
  }
  [[nodiscard]] auto getType() const -> TypeRef {
    bort_assert(!isShallow(), "Type access on shallow symbol");
    return m_Type;
  }

  [[nodiscard]] auto toString() const -> std::string override;

private:
  TypeRef m_Type;
};

class Function final : public Symbol {
public:
  explicit Function(std::string name)
      : Symbol{ ObjectKind::Function, std::move(name), true },
        m_ReturnType{ nullptr } {
  }
  Function(std::string name, TypeRef returnType,
           std::initializer_list<TypeRef> argTypes)
      : Symbol{ ObjectKind::Function, std::move(name), false },
        m_ReturnType{ std::move(returnType) },
        m_ArgTypes{ argTypes } {
  }

  [[nodiscard]] auto toString() const -> std::string override;

private:
  TypeRef m_ReturnType;
  std::vector<TypeRef> m_ArgTypes;
};
} // namespace bort