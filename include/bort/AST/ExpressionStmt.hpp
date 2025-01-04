#pragma once
#include "bort/AST/ASTNode.hpp"
#include "bort/AST/ExpressionNode.hpp"

namespace bort::ast {

class ExpressionStmt : public Statement {
  explicit ExpressionStmt(Ref<ExpressionNode> expr)
      : Statement{ NodeKind::ExpressionStmt },
        m_Expression{ std::move(expr) } {
  }

public:
  auto getExpression() -> Ref<ExpressionNode> {
    return m_Expression;
  }

  friend class ASTRoot;

private:
  Ref<ExpressionNode> m_Expression;
};

} // namespace bort::ast
