#include "bort/AST/BinOpExpr.hpp"
#include "bort/AST/DumpCommons.hpp"

namespace bort::ast {

void BinOpExpr::dump(int depth) const {
  ExpressionNode::dump(depth);
  internal::dump(depth, "Op",
                 Token::TokenNameMapping.Find(m_Op).value_or("UNKNOWN"));
  internal::dump(depth, "LHS", m_Lhs.get());
  internal::dump(depth, "RHS", m_Rhs.get());
}

void BinOpExpr::preOrderVisit(
    const std::shared_ptr<ASTVisitor>& visitor) {
  visitor->visit(this);
  m_Lhs->preOrderVisit(visitor);
  m_Rhs->preOrderVisit(visitor);
}
void BinOpExpr::postOrderVisit(
    const std::shared_ptr<ASTVisitor>& visitor) {
  m_Lhs->postOrderVisit(visitor);
  m_Rhs->postOrderVisit(visitor);
  visitor->visit(this);
}

} // namespace bort::ast
