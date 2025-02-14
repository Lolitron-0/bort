#pragma once
#include "bort/AST/ASTNode.hpp"
#include "bort/AST/ExpressionNode.hpp"
#include "bort/Basic/Assert.hpp"

namespace bort::ast {

class ReturnStmt : public Statement {
public:
  ReturnStmt()
      : Statement{ NodeKind::ReturnStmt } {
  }

  explicit ReturnStmt(Ref<ExpressionNode> expr)
      : Statement{ NodeKind::ReturnStmt },
        m_Expression{ std::move(expr) } {
  }

  [[nodiscard]] auto hasExpression() const -> bool {
    return m_Expression != nullptr;
  }

  [[nodiscard]] auto getExpression() const -> Ref<ExpressionNode> {
    bort_assert(hasExpression(),
                "getExpression on empty return statement");
    return m_Expression;
  }

private:
  Ref<ExpressionNode> m_Expression{ nullptr };
};

} // namespace bort::ast
