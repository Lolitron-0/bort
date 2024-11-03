#pragma once
#include "bort/AST/ASTNode.hpp"
#include "bort/Basic/Ref.hpp"
#include "bort/Lex/Lexer.hpp"

namespace bort {

// TODO: don't check null on error and then traverse, checking for
// null, to recieve more errors at once

class Parser {
public:
  explicit Parser(const Ref<TokenList>& tokenList)
      : m_Tokens{ tokenList },
        m_CurTokIter{ tokenList->begin() },
        m_ASTRoot{ makeUnique<ast::ASTRoot>() } {
  }

  auto buildAST() -> Ref<ast::ASTRoot>;

private:
  [[nodiscard]] inline auto curTok() -> const Token& {
    return *m_CurTokIter;
  }

  inline void consumeToken() {
    m_CurTokIter++;
  }

  // number -> {integer}
  auto parseNumberExpr() -> Unique<ast::NumberExpr>;
  // parenExpr -> '(' expression ')'
  auto parseParenExpr() -> Unique<ast::ExpressionNode>;
  // identifier
  // -> identifier - variable
  // -> identifier '(' expr, ... ')' - function call
  auto parseIdentifierExpr() -> Unique<ast::ExpressionNode>;
  // value expression
  // -> number
  // -> parenExpr
  // -> identifierExpr
  auto parseValueExpression() -> Unique<ast::ExpressionNode>;
  // expression
  // -> valueExpression (binOp valueExpression ...)
  auto parseExpression() -> Unique<ast::ExpressionNode>;
  // binOpRhs
  // -> bipOp valueExpression (binOpRhs ...)
  auto parseBinOpRhs(Unique<ast::ExpressionNode> lhs,
                     int32_t prevPrecedence = 0)
      -> Unique<ast::ExpressionNode>;

  Ref<TokenList> m_Tokens;
  TokenList::const_iterator m_CurTokIter;
  Ref<ast::ASTRoot> m_ASTRoot;
};

} // namespace bort
