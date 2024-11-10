#include "bort/AST/BinOpExpr.hpp"

namespace bort::ast {

BinOpExpr::BinOpExpr(Ref<ExpressionNode> lhs, Ref<ExpressionNode> rhs,
                     TokenKind op)
    : ExpressionNode{ NodeKind::BinOpExpr, nullptr },
      m_Op{ op },
      m_Lhs{ std::move(lhs) },
      m_Rhs{ std::move(rhs) } {
}

auto BinOpExpr::getOp() const -> TokenKind {
  return m_Op;
}

auto BinOpExpr::getLhs() -> Ref<ExpressionNode> {
  return m_Lhs;
}

auto BinOpExpr::getRhs() -> Ref<ExpressionNode> {
  return m_Rhs;
}

} // namespace bort::ast
