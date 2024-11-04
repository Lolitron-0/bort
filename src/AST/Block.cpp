#include "bort/AST/Block.hpp"

namespace bort::ast {

void Block::preOrderVisit(const Ref<ASTVisitor>& visitor) {
  visitor->visit(this);
  for (auto&& child : m_Body) {
    child->preOrderVisit(visitor);
  }
}
void Block::postOrderVisit(const Ref<ASTVisitor>& visitor) {
  for (auto&& child : m_Body) {
    child->postOrderVisit(visitor);
  }
  visitor->visit(this);
}

} // namespace bort::ast
