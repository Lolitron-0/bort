#include "bort/Parse/Parser.hpp"
#include "bort/AST/ASTDebugInfo.hpp"
#include "bort/AST/ASTNode.hpp"
#include "bort/AST/BinOpExpr.hpp"
#include "bort/AST/Block.hpp"
#include "bort/AST/ExpressionStmt.hpp"
#include "bort/AST/NumberExpr.hpp"
#include "bort/AST/VarDecl.hpp"
#include "bort/AST/VariableExpr.hpp"
#include "bort/AST/WhileStmt.hpp"
#include "bort/Basic/Assert.hpp"
#include "bort/Basic/Ref.hpp"
#include "bort/CLI/IO.hpp"
#include "bort/Frontend/Type.hpp"
#include "bort/Lex/Token.hpp"
#include <frozen/unordered_map.h>

namespace bort {

static constexpr auto s_BinopPrecedence{
  frozen::make_unordered_map<TokenKind, int32_t>({
      { TokenKind::Plus, 20 },
      { TokenKind::Minus, 20 },
      { TokenKind::Star, 40 },
      { TokenKind::Div, 40 },
      { TokenKind::Less, 10 },
      { TokenKind::Assign, 5 },
  })
};

/// we do not support user defined aggregates or typedefs, so just check
/// for builtin type or declspec
static constexpr auto isTypenameStart(const Token& tok) -> bool {
  return tok.isOneOf(TokenKind::KW_const, TokenKind::KW_int,
                     TokenKind::KW_void, TokenKind::KW_char);
}

auto Parser::buildAST() -> Ref<ast::ASTRoot> {
  while (true) {
    if (curTok().is(TokenKind::Eof)) {
      break;
    }

    auto node{ parseStatement() };
    if (!node) {
      break;
    }
    m_ASTRoot->pushChild(node);
  }
  return m_ASTRoot;
}

auto Parser::parseNumberExpr() -> Unique<ast::NumberExpr> {
  bort_assert(curTok().is(TokenKind::NumericLiteral),
              "Expected numeric literal");
  auto result{ curTok().getLiteralValue<ast::NumberExpr::ValueT>() };
  auto token{ curTok() };
  consumeToken();
  return m_ASTRoot->registerNode<ast::NumberExpr>(
      ast::ASTDebugInfo{ token }, result);
}

auto Parser::parseParenExpr() -> Unique<ast::ExpressionNode> {
  bort_assert(curTok().is(TokenKind::LParen), "Expected '('");
  consumeToken();

  auto result{ parseExpression() };
  if (!result) {
    return invalidNode();
  }

  if (curTok().isNot(TokenKind::RParen)) {
    Diagnostic::emitError(curTok(), "Expected ')'");
    return invalidNode();
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
        ast::ASTDebugInfo{ identifierTok },
        makeRef<Variable>(std::string{ identifierTok.getStringView() }));
  }

  /// @todo parse function call
  return invalidNode();
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
    Diagnostic::emitError(curTok(), "Expected value expression");
    return invalidNode();
  }
}

auto Parser::parseExpression() -> std::unique_ptr<ast::ExpressionNode> {
  auto lhs{ parseValueExpression() };
  if (!lhs) {
    return invalidNode();
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
      return invalidNode();
    }

    // now: lhs binOp rhs unparsed

    auto nextPrecedence{ getTokPrecedence(curTok()) };
    // if associates to the right: lhs binOp (rhs lookahead unparsed)
    if (tokPrecedence < nextPrecedence) {
      rhs = parseBinOpRhs(std::move(rhs), tokPrecedence + 1);
      if (!rhs) {
        return invalidNode();
      }
    }

    // now: (lhs binOp rhs) lookahead unparsed
    lhs = m_ASTRoot->registerNode<ast::BinOpExpr>(
        ast::ASTDebugInfo{ binOp }, std::move(lhs), std::move(rhs),
        binOp.getKind());
  }
}

auto Parser::parseDeclspec() -> TypeRef {
  bort_assert(isTypenameStart(curTok()), "Expected type name");
  auto kind{ curTok().getKind() };
  consumeToken();
  Ref<Type> type;

  switch (kind) {
  case TokenKind::KW_int:
    type = IntType::get();
    break;
  case TokenKind::KW_char:
    type = CharType::get();
    break;
  case TokenKind::KW_void:
    type = VoidType::get();
    break;
  case TokenKind::KW_const:
    bort_assert(false, "Not implemented");
  default:
    bort_assert(false, "Unreachable");
  }

  while (curTok().is(TokenKind::Star)) {
    consumeToken();
    type = PointerType::get(type);
  }

  return type;
}

auto Parser::parseDeclaration() -> Ref<ast::Statement> {
  auto type{ parseDeclspec() };
  if (curTok().isNot(TokenKind::Identifier)) {
    Diagnostic::emitError(curTok(), "Expected variable name");
    return invalidNode();
  }
  auto nameTok{ curTok() };
  consumeToken();

  if (curTok().is(TokenKind::LParen)) {
    return parseFunctionDecl(type, nameTok);
  }

  return parseVarDecl(type, nameTok);
}

auto Parser::parseVarDecl(const TypeRef& type,
                          const Token& nameTok) -> Ref<ast::VarDecl> {
  std::string name{ nameTok.getStringView() };
  auto node{ m_ASTRoot->registerNode<ast::VarDecl>(
      ast::ASTDebugInfo{ nameTok }, type, name) };

  if (curTok().isNot(TokenKind::Semicolon)) {
    Diagnostic::emitError(curTok(), "Expected ';'");
    return invalidNode();
  }
  consumeToken();

  if (type->getKind() == TypeKind::Void) {
    Diagnostic::emitError(nameTok, "Variable of incomplete type 'void'");
    return invalidNode();
  }
  return node;
}

auto Parser::parseFunctionDecl(const TypeRef& type, const Token& nameTok)
    -> Ref<ast::FunctionDecl> {
  bort_assert(curTok().is(TokenKind::LParen), "Expected '('");
  consumeToken();

  std::string name{ nameTok.getStringView() };
  std::vector<Variable> args;

  while (curTok().isNot(TokenKind::RParen)) {
    auto argType{ parseDeclspec() };

    if (argType && argType->is(TypeKind::Void)) {
      Diagnostic::emitError(curTok(),
                            "Parameter of incomplete type 'void'");
      return invalidNode();
    }

    if (curTok().isNot(TokenKind::Identifier)) {
      Diagnostic::emitError(curTok(), "All parameters should be named");
      return invalidNode();
    }

    auto argNameTok{ curTok() };
    consumeToken();

    if (curTok().isNot(TokenKind::Comma)) {
      Diagnostic::emitError(curTok(), "Expected ','");
      return invalidNode();
    }
    consumeToken();

    args.emplace_back(std::string{ argNameTok.getStringView() }, argType);
  }
  consumeToken();

  if (curTok().isNot(TokenKind::LBrace)) {
    Diagnostic::emitError(
        curTok(), "Expected '{}' (all functions must be definitions)",
        '{');
    return invalidNode();
  }
  auto block{ parseBlock() };
  return m_ASTRoot->registerNode<ast::FunctionDecl>(
      ast::ASTDebugInfo{ nameTok }, std::move(name), type,
      std::move(args), std::move(block));
}

auto Parser::parseStatement() -> Ref<ast::Statement> {
  if (curTok().is(TokenKind::LBrace)) {
    return parseBlock();
  }

  if (isTypenameStart(curTok())) {
    return parseDeclaration();
  }

  switch (curTok().getKind()) {
  case TokenKind::KW_if:
    return parseIfStatement();
  case TokenKind::KW_while:
    return parseWhileStatement();
  default:
    break;
  }

  // it's an expression statement
  auto stmtTok{ curTok() };
  auto expression{ parseExpression() };
  if (curTok().isNot(TokenKind::Semicolon)) {
    Diagnostic::emitError(curTok(), "Expected ';'");
    return invalidNode();
  }
  consumeToken();
  return m_ASTRoot->registerNode<ast::ExpressionStmt>(
      ast::ASTDebugInfo{ stmtTok }, std::move(expression));
}

auto Parser::parseBlock() -> Unique<ast::Block> {
  bort_assert(curTok().is(TokenKind::LBrace), "Expected '{'");
  auto block{ m_ASTRoot->registerNode<ast::Block>(
      ast::ASTDebugInfo{ curTok() }) };
  consumeToken();

  Ref<ast::Node> child{ nullptr };
  while (!curTok().is(TokenKind::RBrace)) {
    child = parseStatement();
    while (!child) {
      consumeToken();
      Diagnostic::setLevel(Diagnostic::Level::Silent);
      child = parseStatement();
      Diagnostic::setLevel(Diagnostic::Level::All);
    }
    block->pushChild(child);
  }

  consumeToken();

  return block;
}

auto Parser::lookahead(uint32_t offset) const -> const Token& {
  auto iter{ m_CurTokIter };
  std::advance(iter, offset);
  return *iter;
}

auto Parser::parseIfStatement() -> Ref<ast::IfStmt> {
  bort_assert(curTok().is(TokenKind::KW_if), "Expected 'if'");
  auto ifTok{ curTok() };
  consumeToken();
  if (curTok().isNot(TokenKind::LParen)) {
    Diagnostic::emitError(curTok(), "Expected '('");
    return invalidNode();
  }
  auto condition{ parseParenExpr() };

  auto thenBlock{ parseBlock() };
  Unique<ast::Block> elseBlock{ nullptr };
  if (curTok().is(TokenKind::KW_else)) {
    consumeToken();
    elseBlock = parseBlock();
  }

  if (!elseBlock) {
    elseBlock = m_ASTRoot->registerNode<ast::Block>(
        ast::ASTDebugInfo{ curTok() });
  }

  return m_ASTRoot->registerNode<ast::IfStmt>(
      ast::ASTDebugInfo{ ifTok }, std::move(condition),
      std::move(thenBlock), std::move(elseBlock));
}

auto Parser::parseWhileStatement() -> Ref<ast::WhileStmt> {
  bort_assert(curTok().is(TokenKind::KW_while), "Expected 'while'");
  auto whileTok{ curTok() };
  consumeToken();
  if (curTok().isNot(TokenKind::LParen)) {
    Diagnostic::emitError(curTok(), "Expected '('");
    return invalidNode();
  }
  auto condition{ parseParenExpr() };
  auto body{ parseBlock() };

  return m_ASTRoot->registerNode<ast::WhileStmt>(
      ast::ASTDebugInfo{ whileTok }, std::move(condition),
      std::move(body));
}

} // namespace bort
