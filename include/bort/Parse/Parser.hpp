#pragma once
#include "bort/Lex/Lexer.hpp"
#include "bort/Parse/ASTNode.hpp"
#include <memory>

namespace bort {

// TODO: don't check null on error and then traverse, checking for
// null, to recieve more errors at once

class Parser {
public:
  explicit Parser(const std::shared_ptr<TokenList>& tokenList)
      : m_Tokens{ tokenList },
        m_CurTokIter{ tokenList->begin() } {
  }

  void buildAST();

private:
  [[nodiscard]] inline auto curTok() -> const Token& {
    return *m_CurTokIter;
  }

  inline void consumeToken() {
    m_CurTokIter++;
  }

  // number -> {integer}
  auto parseNumberExpr() -> std::unique_ptr<ast::NumberExpr>;
  // parenExpr -> '(' expression ')'
  auto parseParenExpr() -> std::unique_ptr<ast::ExpressionNode>;
  // identifier
  // -> identifier - variable
  // -> identifier '(' expr, ... ')' - function call
  auto parseIdentifierExpr() -> std::unique_ptr<ast::ExpressionNode>;
  // value expression
  // -> number
  // -> parenExpr
  // -> identifierExpr
  auto parseValueExpression() -> std::unique_ptr<ast::ExpressionNode>;
  // expression
  // -> valueExpression (binOp valueExpression ...)
  auto parseExpression() -> std::unique_ptr<ast::ExpressionNode>;
  // binOpRhs
  // -> bipOp valueExpression (binOpRhs ...)
  auto parseBinOpRhs(std::unique_ptr<ast::ExpressionNode> lhs,
                     int32_t prevPrecedence = 0)
      -> std::unique_ptr<ast::ExpressionNode>;

  std::shared_ptr<TokenList> m_Tokens;
  TokenList::const_iterator m_CurTokIter;
};

} // namespace bort
