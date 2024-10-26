#include "bort/Parse/Parser.hpp"
#include "bort/CLI/IO.hpp"
#include "bort/Lex/Token.hpp"
#include "bort/Parse/ASTNode.hpp"
#include <cassert>
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

void Parser::buildAST() {
  while (true) {
    if (curTok().is(TokenKind::Eof)) {
      break;
    }

    auto expr{ parseExpression() };
    if (!expr) {
      break;
    }
    expr->dump();
  }
}

auto Parser::parseNumberExpr() -> std::unique_ptr<ast::NumberExpr> {
  assert(curTok().is(TokenKind::NumericLiteral) &&
         "Expected numeric literal");
  auto result{ curTok().getLiteralValue<ast::NumberExpr::ValueT>() };
  consumeToken();
  return std::make_unique<ast::NumberExpr>(result);
}

auto Parser::parseParenExpr() -> std::unique_ptr<ast::ExpressionNode> {
  assert(curTok().is(TokenKind::LParen) && "Expected '('");
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

auto Parser::parseIdentifierExpr()
    -> std::unique_ptr<ast::ExpressionNode> {
  assert(curTok().is(TokenKind::Identifier) && "Expected identifier");
  std::string identifierName{ curTok().getString() };
  consumeToken();

  if (curTok().isNot(TokenKind::LParen)) {
    return std::make_unique<ast::VariableExpr>(identifierName);
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
    lhs = std::make_unique<ast::BinOpExpr>(std::move(lhs), std::move(rhs),
                                           binOp.getKind());
  }
}

} // namespace bort
