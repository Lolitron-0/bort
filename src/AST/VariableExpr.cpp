#include "bort/AST/VariableExpr.hpp"

namespace bort::ast {

void VariableExpr::preOrderVisit(
    const std::shared_ptr<ASTVisitor>& visitor) {
  visitor->visit(this);
}
void VariableExpr::postOrderVisit(
    const std::shared_ptr<ASTVisitor>& visitor) {
  visitor->visit(this);
}

} // namespace bort::ast
