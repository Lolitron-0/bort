#pragma once
#include "bort/AST/ExpressionNode.hpp"

namespace bort::ast {

class NumberExpr final : public ExpressionNode {
public:
  // TODO typed constants
  using ValueT = int;

private:
  explicit NumberExpr(ValueT value)
      : ExpressionNode{ NodeKind::NumberExpr, IntType::get() },
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
