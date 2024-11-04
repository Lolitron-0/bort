#include "bort/AST/BinOpExpr.hpp"

namespace bort::ast {

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
