#include <utility>

#include "bort/AST/ASTNode.hpp"
#include "bort/AST/ExpressionNode.hpp"
#include "bort/AST/UnaryOpExpr.hpp"
#include "bort/Lex/Token.hpp"

namespace bort::ast {

UnaryOpExpr::UnaryOpExpr(Ref<ExpressionNode> operand, TokenKind op)
    : ExpressionNode{ NodeKind::UnaryOpExpr, nullptr },
      m_Op{ op },
      m_Operand{ std::move(operand) } {
}

auto UnaryOpExpr::getOp() const -> TokenKind {
  return m_Op;
}

auto UnaryOpExpr::getOperand() -> Ref<ExpressionNode> {
  return m_Operand;
}

void UnaryOpExpr::setOperand(Ref<ExpressionNode> operand) {
  m_Operand = std::move(operand);
}
void UnaryOpExpr::setOp(TokenKind op) {
  m_Op = op;
}
} // namespace bort::ast
