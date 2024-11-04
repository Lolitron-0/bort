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
  void preOrderVisit(const Ref<ASTVisitor>& visitor) override;
  void postOrderVisit(const Ref<ASTVisitor>& visitor) override;

  friend class ASTRoot;

protected:
  void dump(int depth) const override;

private:
  ValueT m_Value;
};

} // namespace bort::ast
