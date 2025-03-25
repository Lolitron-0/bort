#pragma once
#include "bort/AST/ExpressionNode.hpp"
#include "bort/Lex/Token.hpp"

namespace bort::ast {

class UnaryOpExpr final : public ExpressionNode {
  UnaryOpExpr(Ref<ExpressionNode> operand, TokenKind op);

public:
  [[nodiscard]] auto getOp() const -> TokenKind;
  [[nodiscard]] auto getOperand() -> Ref<ExpressionNode>;

  friend class ASTRoot;

private:
  TokenKind m_Op;
  Ref<ExpressionNode> m_Operand;
};

} // namespace bort::ast
