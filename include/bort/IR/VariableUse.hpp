#pragma once
#include "bort/Frontend/Symbol.hpp"
#include "bort/IR/Value.hpp"

namespace bort::ir {
class VariableUse : public Value {
public:
  explicit VariableUse(Ref<Variable> variable)
      : Value{ variable->getType(), variable->getName() },
        m_Variable{ std::move(variable) } {
  }

  static auto createUnique(TypeRef type, std::string name) -> ValueRef;

private:
  Ref<Variable> m_Variable;
  size_t m_NameCounter{ 1 }; ///< Counter for unique names
  static std::unordered_map<std::string, Ref<VariableUse>> s_Uses;
};
} // namespace bort::ir
