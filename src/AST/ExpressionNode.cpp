#include "bort/AST/ExpressionNode.hpp"

namespace bort::ast {

ExpressionNode::ExpressionNode(NodeKind kind)
    : ExpressionNode{ kind, nullptr } {
}

ExpressionNode::ExpressionNode(NodeKind kind, TypeRef type)
    : Node{ kind },
      m_Type{ std::move(type) } {
}

auto ExpressionNode::isTypeResolved() const -> bool {
  return m_Type != nullptr;
}

auto ExpressionNode::getType() const -> TypeRef {
  return m_Type;
}

void ExpressionNode::setType(TypeRef type) {
  m_Type = std::move(type);
}

} // namespace bort::ast
