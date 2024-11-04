#include "bort/AST/ASTNode.hpp"
#include <boost/range.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <cul/BiMap.hpp>

namespace bort::ast {

void ASTRoot::preOrderVisit(const Ref<ASTVisitor>& visitor) {
  visitor->visit(this);
  for (auto&& child : m_Children) {
    child->preOrderVisit(visitor);
  }
}

void ASTRoot::postOrderVisit(const Ref<ASTVisitor>& visitor) {
  for (auto&& child : m_Children) {
    child->postOrderVisit(visitor);
  }
  visitor->visit(this);
}

} // namespace bort::ast
