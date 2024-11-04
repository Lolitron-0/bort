#include "bort/AST/NumberExpr.hpp"

namespace bort::ast {

void NumberExpr::preOrderVisit(
    const std::shared_ptr<ASTVisitor>& visitor) {
  visitor->visit(this);
}
void NumberExpr::postOrderVisit(
    const std::shared_ptr<ASTVisitor>& visitor) {
  visitor->visit(this);
}

} // namespace bort::ast
