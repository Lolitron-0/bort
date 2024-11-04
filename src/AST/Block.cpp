#include "bort/AST/Block.hpp"
#include "bort/AST/DumpCommons.hpp"
#include <boost/range/adaptors.hpp>

namespace bort::ast {

void Block::dump(int depth) const {
  Node::dump(depth);
  for (auto&& el : m_Body | boost::adaptors::indexed()) {
    internal::dump(depth, fmt::format("#{}", el.index()),
                   el.value().get());
  }
}

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
