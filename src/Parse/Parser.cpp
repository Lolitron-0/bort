#include "bort/Parse/Parser.hpp"
#include "bort/AST/ASTNode.hpp"
#include "bort/CLI/IO.hpp"
#include "bort/Lex/Token.hpp"
#include <frozen/unordered_map.h>

namespace bort {

static constexpr auto s_BinopPrecedence{
  frozen::make_unordered_map<TokenKind, int32_t>(
      { { TokenKind::Plus, 20 },
        { TokenKind::Minus, 20 },
        { TokenKind::Star, 40 },
        { TokenKind::Div, 40 },
        { TokenKind::Less, 10 } })
};

auto Parser::buildAST() -> Ref<ast::ASTRoot> {
  while (true) {
    if (curTok().is(TokenKind::Eof)) {
      break;
    }

    Ref<ast::ExpressionNode> expr{ parseExpression() };
    if (!expr) {
      break;
    }
    m_ASTRoot->pushChild(expr);
  }
  return m_ASTRoot;
}

auto Parser::parseNumberExpr() -> Unique<ast::NumberExpr> {
  bort_assert(curTok().is(TokenKind::NumericLiteral),
              "Expected numeric literal");
  auto result{ curTok().getLiteralValue<ast::NumberExpr::ValueT>() };
  auto token{ curTok() };
  consumeToken();
  return m_ASTRoot->registerNode<ast::NumberExpr>(token, result);
}

auto Parser::parseParenExpr() -> Unique<ast::ExpressionNode> {
  bort_assert(curTok().is(TokenKind::LParen), "Expected '('");
  consumeToken();
  auto result{ parseExpression() };
  if (!result) {
    return nullptr;
  }

  if (curTok().isNot(TokenKind::RParen)) {
    emitError(curTok(), "Expected ')'");
    return nullptr;
  }

  consumeToken();
  return result;
}

auto Parser::parseIdentifierExpr() -> Unique<ast::ExpressionNode> {
  bort_assert(curTok().is(TokenKind::Identifier), "Expected identifier");
  auto identifierTok{ curTok() };

  consumeToken();

  if (curTok().isNot(TokenKind::LParen)) {
    // it's a variable
    return m_ASTRoot->registerNode<ast::VariableExpr>(
        identifierTok,
        makeRef<Variable>(std::string{ identifierTok.getStringView() }));
  }

  // TODO: parse function call
  return nullptr;
}

auto Parser::parseValueExpression()
    -> std::unique_ptr<ast::ExpressionNode> {
  switch (curTok().getKind()) {
  case TokenKind::Identifier:
    return parseIdentifierExpr();
  case TokenKind::LParen:
    return parseParenExpr();
  case TokenKind::NumericLiteral:
    return parseNumberExpr();
  default:
    emitError(curTok(), "Expected value expression");
    return nullptr;
  }
}

auto Parser::parseExpression() -> std::unique_ptr<ast::ExpressionNode> {
  auto lhs{ parseValueExpression() };
  if (!lhs) {
    return nullptr;
  }
  return parseBinOpRhs(std::move(lhs));
}

static auto getTokPrecedence(const Token& tok) -> int32_t {
  if (s_BinopPrecedence.contains(tok.getKind())) {
    return s_BinopPrecedence.at(tok.getKind());
  }
  return -1;
}

auto Parser::parseBinOpRhs(std::unique_ptr<ast::ExpressionNode> lhs,
                           int32_t prevPrecedence)
    -> std::unique_ptr<ast::ExpressionNode> {
  while (true) {
    // precedence or -1 if not a binop
    auto tokPrecedence{ getTokPrecedence(curTok()) };
    if (tokPrecedence < prevPrecedence) {
      return lhs;
    }

    auto binOp{ curTok() };
    consumeToken();
    auto rhs{ parseValueExpression() };
    if (!rhs) {
      return nullptr;
    }

    // now: lhs binOp rhs unparsed

    auto nextPrecedence{ getTokPrecedence(curTok()) };
    // if associates to the right: lhs binOp (rhs lookahead unparsed)
    if (tokPrecedence < nextPrecedence) {
      rhs = parseBinOpRhs(std::move(rhs), tokPrecedence + 1);
      if (!rhs) {
        return nullptr;
      }
    }

    // now: (lhs binOp rhs) lookahead unparsed
    lhs = m_ASTRoot->registerNode<ast::BinOpExpr>(
        binOp, std::move(lhs), std::move(rhs), binOp.getKind());
  }
}

} // namespace bort
