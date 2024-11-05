#pragma once
#include "bort/AST/ExpressionNode.hpp"

namespace bort::ast {

class VariableExpr final : public ExpressionNode {
  explicit VariableExpr(const Ref<Symbol>& variable)
      : ExpressionNode{ NodeKind::VariableExpr } {
    bort_assert((variable->getKind() == ObjectKind::Variable),
                "Variable expr got not variable object");
    m_Variable = std::dynamic_pointer_cast<Variable>(variable);
    m_Type = m_Variable->isShallow() ? nullptr : m_Variable->getType();
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
    m_Type = m_Variable->isShallow() ? nullptr : m_Variable->getType();
  }

  friend class ASTRoot;

private:
  Ref<Variable> m_Variable;
};

} // namespace bort::ast
