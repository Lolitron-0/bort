#include "bort/AST/VariableExpr.hpp"

namespace bort::ast {

VariableExpr::VariableExpr(Ref<Variable> variable)
    : ExpressionNode{ NodeKind::VariableExpr } {
  setVariable(std::move(variable));
}

auto VariableExpr::getVariable() -> Ref<Variable> {
  return m_Variable;
}

auto VariableExpr::getVarName() const -> std::string {
  return m_Variable->getName();
}

auto VariableExpr::isResolved() const -> bool {
  return !m_Variable->isShallow();
}

void VariableExpr::setVariable(Ref<Variable> variable) {
  m_Variable = std::move(variable);
  setType(m_Variable->isShallow() ? nullptr : m_Variable->getType());
}

} // namespace bort::ast
