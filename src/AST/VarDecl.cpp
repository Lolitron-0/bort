#include "bort/AST/VarDecl.hpp"
#include <boost/range/adaptors.hpp>

namespace bort::ast {

void VarDecl::dump(int depth) const {
  Node::dump(depth);
  internal::dump(depth, "Name", m_Variable->getName());
  internal::dump(depth, "Type", m_Variable->getType()->toString());
}

void ASTRoot::dump(int depth) const {
  Node::dump(depth);
  for (auto&& el : m_Children | boost::adaptors::indexed()) {
    internal::dump(depth - 1, fmt::format("#{}", el.index()),
                   el.value().get());
  }
}

void VarDecl::preOrderVisit(const Ref<ASTVisitor>& visitor) {
  visitor->visit(this);
}
void VarDecl::postOrderVisit(const Ref<ASTVisitor>& visitor) {
  visitor->visit(this);
}

} // namespace bort::ast
