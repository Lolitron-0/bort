#pragma once
#include "bort/AST/ExpressionNode.hpp"

namespace bort::ast {

class BinOpExpr final : public ExpressionNode {
  BinOpExpr(Ref<Node> lhs, Ref<Node> rhs, TokenKind op)
      : ExpressionNode{ NodeKind::BinOpExpr, nullptr },
        m_Op{ op },
        m_Lhs{ std::move(lhs) },
        m_Rhs{ std::move(rhs) } {
  }

public:
  void preOrderVisit(const Ref<ASTVisitor>& visitor) override;
  void postOrderVisit(const Ref<ASTVisitor>& visitor) override;

  [[nodiscard]] auto getOp() const -> TokenKind {
    return m_Op;
  }
  [[nodiscard]] auto getLhs() -> Ref<Node> {
    return m_Lhs;
  }
  [[nodiscard]] auto getRhs() -> Ref<Node> {
    return m_Rhs;
  }

  friend class ASTRoot;

protected:
  void dump(int depth) const override;

private:
  TokenKind m_Op;
  Ref<Node> m_Lhs;
  Ref<Node> m_Rhs;
};

} // namespace bort::ast
