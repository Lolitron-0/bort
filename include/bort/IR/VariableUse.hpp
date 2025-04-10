#pragma once
#include "bort/Frontend/Symbol.hpp"
#include "bort/IR/Value.hpp"

namespace bort::ir {
/// @brief Operand wrapper that represents usage of a variable in
/// instruction
class VariableUse final : public Operand {
public:
  static auto createUnique(const Ref<Variable>& variable) -> ValueRef;
  static auto get(const Ref<Variable>& variable) -> ValueRef;

  [[nodiscard]] auto getVariable() const -> Ref<Variable> {
    return m_Variable;
  }

private:
  explicit VariableUse(Ref<Variable> variable, std::string name)
      : Operand{ variable->getType(), std::move(name) },
        m_Variable{ std::move(variable) } {
  }

private:
  Ref<Variable> m_Variable;

  size_t m_NameCounter{ 1 }; ///< Counter for unique names
  static std::unordered_map<Ref<Variable>, Ref<VariableUse>> s_Uses;
};
} // namespace bort::ir
