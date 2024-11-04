#include "bort/AST/NumberExpr.hpp"
#include "bort/AST/DumpCommons.hpp"

namespace bort::ast {

void NumberExpr::dump(int depth) const {
  ExpressionNode::dump(depth);
  internal::dump(depth, "Value", m_Value);
}

void NumberExpr::preOrderVisit(
    const std::shared_ptr<ASTVisitor>& visitor) {
  visitor->visit(this);
}
void NumberExpr::postOrderVisit(
    const std::shared_ptr<ASTVisitor>& visitor) {
  visitor->visit(this);
}

} // namespace bort::ast
