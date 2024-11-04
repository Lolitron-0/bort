#include "bort/AST/ExpressionNode.hpp"
#include "bort/AST/DumpCommons.hpp"

namespace bort::ast {

void ExpressionNode::dump(int depth) const {
  Node::dump(depth);
  internal::dump(depth, "Type",
                 isTypeResolved() ? m_Type->toString()
                                  : red("unresolved"));
}

} // namespace bort::ast
