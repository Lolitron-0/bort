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

auto BinOpExpr::getLHS() -> Ref<ExpressionNode> {
  return m_Lhs;
}

auto BinOpExpr::getRHS() -> Ref<ExpressionNode> {
  return m_Rhs;
}

void BinOpExpr::setLHS(Ref<ExpressionNode> lhs) {
  m_Lhs = std::move(lhs);
}
void BinOpExpr::setRHS(Ref<ExpressionNode> rhs) {
  m_Rhs = std::move(rhs);
}
} // namespace bort::ast
