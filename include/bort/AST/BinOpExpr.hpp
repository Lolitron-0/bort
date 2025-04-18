#pragma once
#include "bort/AST/ExpressionNode.hpp"
#include "bort/Lex/Token.hpp"

namespace bort::ast {

class BinOpExpr final : public ExpressionNode {
  BinOpExpr(Ref<ExpressionNode> lhs, Ref<ExpressionNode> rhs,
            TokenKind op);

public:
  [[nodiscard]] auto getOp() const -> TokenKind;
  [[nodiscard]] auto getLHS() -> Ref<ExpressionNode>;
  [[nodiscard]] auto getRHS() -> Ref<ExpressionNode>;
  [[nodiscard]] constexpr auto isArithmetic() const -> bool {
    /// @todo bitwise operators
    return m_Op == TokenKind::Plus || m_Op == TokenKind::Minus ||
           m_Op == TokenKind::Star || m_Op == TokenKind::Div ||
           m_Op == TokenKind::Mod || m_Op == TokenKind::LShift ||
           m_Op == TokenKind::RShift || m_Op == TokenKind::Amp ||
           m_Op == TokenKind::Pipe || m_Op == TokenKind::Xor ||
           m_Op == TokenKind::Amp || m_Op == TokenKind::Pipe;
  }
  [[nodiscard]] constexpr auto isLogical() const -> bool {
    return m_Op == TokenKind::Less || m_Op == TokenKind::Greater ||
           m_Op == TokenKind::LessEqual ||
           m_Op == TokenKind::GreaterEqual || m_Op == TokenKind::Equals;
  }

  void setLHS(Ref<ExpressionNode> lhs);
  void setRHS(Ref<ExpressionNode> rhs);

  friend class ASTRoot;

private:
  TokenKind m_Op;
  Ref<ExpressionNode> m_Lhs;
  Ref<ExpressionNode> m_Rhs;
};

} // namespace bort::ast
