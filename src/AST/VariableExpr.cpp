#include "bort/AST/VariableExpr.hpp"
#include "bort/AST/DumpCommons.hpp"

namespace bort::ast {

void VariableExpr::dump(int depth) const {
  ExpressionNode::dump(depth);
  internal::dump(depth, "Variable ",
                 isResolved() ? m_Variable->getName()
                              : red("unresolved"));
}

void VariableExpr::preOrderVisit(
    const std::shared_ptr<ASTVisitor>& visitor) {
  visitor->visit(this);
}
void VariableExpr::postOrderVisit(
    const std::shared_ptr<ASTVisitor>& visitor) {
  visitor->visit(this);
}

} // namespace bort::ast
