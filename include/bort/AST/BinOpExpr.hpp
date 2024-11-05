#pragma once
#include "bort/AST/ExpressionNode.hpp"

namespace bort::ast {

class BinOpExpr final : public ExpressionNode {
  BinOpExpr(Ref<ExpressionNode> lhs, Ref<ExpressionNode> rhs, TokenKind op)
      : ExpressionNode{ NodeKind::BinOpExpr, nullptr },
        m_Op{ op },
        m_Lhs{ std::move(lhs) },
        m_Rhs{ std::move(rhs) } {
  }

public:
  [[nodiscard]] auto getOp() const -> TokenKind {
    return m_Op;
  }
  [[nodiscard]] auto getLhs() -> Ref<ExpressionNode> {
    return m_Lhs;
  }
  [[nodiscard]] auto getRhs() -> Ref<ExpressionNode> {
    return m_Rhs;
  }

  friend class ASTRoot;

private:
  TokenKind m_Op;
  Ref<ExpressionNode> m_Lhs;
  Ref<ExpressionNode> m_Rhs;
};

} // namespace bort::ast
