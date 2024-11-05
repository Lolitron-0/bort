#pragma once
#include "bort/AST/ASTNode.hpp"

namespace bort::ast {

class ExpressionNode : public Node {
protected:
  // in case expression type is not yet known
  explicit ExpressionNode(NodeKind kind)
      : ExpressionNode{ kind, nullptr } {
  }
  ExpressionNode(NodeKind kind, TypeRef type)
      : Node{ kind },
        m_Type{ std::move(type) } {
  }

public:
  [[nodiscard]] auto isTypeResolved() const -> bool {
    return m_Type != nullptr;
  }

  [[nodiscard]] auto getType() const -> TypeRef {
    return m_Type;
  }

  void setType(TypeRef type) {
    m_Type = std::move(type);
  }

protected:
  TypeRef m_Type;
};

} // namespace bort::ast
