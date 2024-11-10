#pragma once
#include "bort/AST/ASTNode.hpp"
#include "bort/Frontend/Type.hpp"

namespace bort::ast {

class ExpressionNode : public Node {
protected:
  // in case expression type is not yet known
  explicit ExpressionNode(NodeKind kind);
  ExpressionNode(NodeKind kind, TypeRef type);

public:
  [[nodiscard]] auto isTypeResolved() const -> bool;

  [[nodiscard]] auto getType() const -> TypeRef;

  void setType(TypeRef type);

protected:
  TypeRef m_Type;
};

} // namespace bort::ast
