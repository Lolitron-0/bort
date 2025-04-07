#pragma once
#include "bort/AST/ExpressionNode.hpp"
#include "bort/Frontend/Symbol.hpp"

namespace bort::ast {

class VariableExpr final : public ExpressionNode {
  explicit VariableExpr(Ref<Variable> variable);

public:
  [[nodiscard]] auto getVariable() -> Ref<Variable>;
  [[nodiscard]] auto getVarName() const -> std::string;

  [[nodiscard]] auto isResolved() const -> bool;
  void setVariable(Ref<Variable> variable);

  friend class ASTRoot;

private:
  Ref<Variable> m_Variable;
};

} // namespace bort::ast
