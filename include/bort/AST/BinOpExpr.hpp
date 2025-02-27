#pragma once
#include "bort/AST/ExpressionNode.hpp"
#include "bort/Lex/Token.hpp"

namespace bort::ast {

class BinOpExpr final : public ExpressionNode {
  BinOpExpr(Ref<ExpressionNode> lhs, Ref<ExpressionNode> rhs,
            TokenKind op);

public:
  [[nodiscard]] auto getOp() const -> TokenKind;
  [[nodiscard]] auto getLhs() -> Ref<ExpressionNode>;
  [[nodiscard]] auto getRhs() -> Ref<ExpressionNode>;
  [[nodiscard]] constexpr auto isArithmetic() const -> bool {
    /// @todo bitwise operators
    return m_Op == TokenKind::Plus || m_Op == TokenKind::Minus ||
           m_Op == TokenKind::Star || m_Op == TokenKind::Div ||
           m_Op == TokenKind::Amp || m_Op == TokenKind::Pipe;
  }
  [[nodiscard]] constexpr auto isLogical() const -> bool {
    return m_Op == TokenKind::Less || m_Op == TokenKind::Greater ||
           m_Op == TokenKind::Equals;
  }

  friend class ASTRoot;

private:
  TokenKind m_Op;
  Ref<ExpressionNode> m_Lhs;
  Ref<ExpressionNode> m_Rhs;
};

} // namespace bort::ast
