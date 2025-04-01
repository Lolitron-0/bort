#pragma once
#include "bort/AST/ExpressionNode.hpp"

namespace bort::ast {

/// Generic node for both char and int constants
class NumberExpr final : public ExpressionNode {
public:
  // both will fit
  using ValueT = int32_t;

private:
  explicit NumberExpr(ValueT value, TypeRef type)
      : ExpressionNode{ NodeKind::NumberExpr, std::move(type) },
        m_Value{ value } {
  }

public:
  [[nodiscard]] auto getValue() const -> ValueT {
    return m_Value;
  }

  friend class ASTRoot;

private:
  ValueT m_Value;
};

} // namespace bort::ast
