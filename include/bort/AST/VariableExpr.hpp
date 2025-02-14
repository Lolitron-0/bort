#pragma once
#include "bort/AST/ExpressionNode.hpp"
#include "bort/Frontend/Symbol.hpp"

namespace bort::ast {

class VariableExpr final : public ExpressionNode {
  explicit VariableExpr(Ref<Variable> variable)
      : ExpressionNode{ NodeKind::VariableExpr } {
    setVariable(std::move(variable));
  }

public:
  [[nodiscard]] auto getVariable() -> Ref<Variable> {
    return m_Variable;
  }
  [[nodiscard]] auto getVarName() const -> std::string {
    return m_Variable->getName();
  }

  [[nodiscard]] auto isResolved() const -> bool {
    return !m_Variable->isShallow();
  }
  void setVariable(Ref<Variable> variable) {
    m_Variable = std::move(variable);
    setType(m_Variable->isShallow() ? nullptr : m_Variable->getType());
  }

  friend class ASTRoot;

private:
  Ref<Variable> m_Variable;
};

} // namespace bort::ast
