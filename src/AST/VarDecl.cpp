#include "bort/AST/VarDecl.hpp"

namespace bort::ast {

void VarDecl::preOrderVisit(const Ref<ASTVisitor>& visitor) {
  visitor->visit(this);
}
void VarDecl::postOrderVisit(const Ref<ASTVisitor>& visitor) {
  visitor->visit(this);
}

} // namespace bort::ast
