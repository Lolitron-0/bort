#include "bort/Parse/Parser.hpp"
#include "bort/AST/ASTDebugInfo.hpp"
#include "bort/AST/ASTNode.hpp"
#include "bort/AST/BinOpExpr.hpp"
#include "bort/AST/Block.hpp"
#include "bort/AST/ExpressionNode.hpp"
#include "bort/AST/ExpressionStmt.hpp"
#include "bort/AST/FunctionCallExpr.hpp"
#include "bort/AST/IndexationExpr.hpp"
#include "bort/AST/InitializerList.hpp"
#include "bort/AST/NumberExpr.hpp"
#include "bort/AST/ReturnStmt.hpp"
#include "bort/AST/UnaryOpExpr.hpp"
#include "bort/AST/VarDecl.hpp"
#include "bort/AST/VariableExpr.hpp"
#include "bort/AST/WhileStmt.hpp"
#include "bort/Basic/Assert.hpp"
#include "bort/Basic/Casts.hpp"
#include "bort/Basic/Ref.hpp"
#include "bort/CLI/IO.hpp"
#include "bort/Frontend/Symbol.hpp"
#include "bort/Frontend/Type.hpp"
#include "bort/Lex/Token.hpp"
#include <frozen/unordered_map.h>

namespace bort {

static constexpr auto s_BinopPrecedence{
  frozen::make_unordered_map<TokenKind, int32_t>({
      { TokenKind::Plus, 200 },      { TokenKind::Minus, 200 },
      { TokenKind::Star, 400 },      { TokenKind::Div, 400 },
      { TokenKind::Less, 100 },      { TokenKind::Greater, 100 },
      { TokenKind::LessEqual, 100 }, { TokenKind::GreaterEqual, 100 },
      { TokenKind::Equals, 95 },     { TokenKind::NotEquals, 95 },
      { TokenKind::Amp, 90 },        { TokenKind::Xor, 80 },
      { TokenKind::Pipe, 70 },       { TokenKind::AmpAmp, 65 },
      { TokenKind::PipePipe, 60 },   { TokenKind::Assign, 50 },
      { TokenKind::PlusAssign, 50 }, { TokenKind::MinusAssign, 50 },
      { TokenKind::StarAssign, 50 }, { TokenKind::DivAssign, 50 },
      { TokenKind::AmpAssign, 50 },  { TokenKind::XorAssign, 50 },
      { TokenKind::PipeAssign, 50 },
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

    auto node{ parseDeclarationStatement() };
    if (!node) {
      break;
    }
    m_ASTRoot->pushChild(node);
  }
  return m_ASTRoot;
}

auto Parser::parseNumberExpr() -> Unique<ast::NumberExpr> {
  bort_assert(
      curTok().isOneOf(TokenKind::NumericLiteral, TokenKind::CharLiteral),
      "Expected literal");
  ast::NumberExpr::ValueT value{};
  TypeRef type;
  if (curTok().is(TokenKind::NumericLiteral)) {
    value = curTok().getLiteralValue<int>();
    type  = IntType::get();
  } else {
    value = curTok().getLiteralValue<char>(); // NOLINT
    type  = CharType::get();
  }
  auto token{ curTok() };
  consumeToken();
  return m_ASTRoot->registerNode<ast::NumberExpr>(
      ast::ASTDebugInfo{ token }, value, std::move(type));
}

auto Parser::parseParenExpr() -> Ref<ast::ExpressionNode> {
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

  if (curTok().is(TokenKind::LParen)) {
    return parseFunctionCallExpr(identifierTok);
  }

  if (curTok().is(TokenKind::LBracket)) {
    return parseIndexationExpr(identifierTok);
  }

  // it's a variable
  return m_ASTRoot->registerNode<ast::VariableExpr>(
      ast::ASTDebugInfo{ identifierTok },
      makeRef<Variable>(std::string{ identifierTok.getStringView() }));
}

auto Parser::parseVarExpr() -> Unique<ast::ExpressionNode> {
  bort_assert(curTok().is(TokenKind::Identifier), "Expected identifier");
  auto varTok{ curTok() };
  consumeToken();

  return m_ASTRoot->registerNode<ast::VariableExpr>(
      ast::ASTDebugInfo{ varTok },
      makeRef<Variable>(std::string{ varTok.getStringView() }));
}

auto Parser::parseValueExpression() -> Ref<ast::ExpressionNode> {

  switch (curTok().getKind()) {
  case TokenKind::Identifier:
    return parseIdentifierExpr();
  case TokenKind::LParen:
    return parseParenExpr();
  case TokenKind::NumericLiteral:
  case TokenKind::CharLiteral:
    return parseNumberExpr();
  case TokenKind::KW_sizeof:
    return parseSizeofExpr();
  default:
    return parseUnaryOpExpr();
  }
}

auto Parser::parseSizeofExpr() -> Unique<ast::ExpressionNode> {
  bort_assert(curTok().is(TokenKind::KW_sizeof), "Expected 'sizeof'");
  auto sizeofTok{ curTok() };
  consumeToken();
  if (curTok().isNot(TokenKind::LParen)) {
    Diagnostic::emitError(curTok(), "Expected '('");
    return invalidNode();
  }

  if (isTypenameStart(lookahead(1))) {
    consumeToken(); // consume lparen
    auto type{ parseDeclspec() };
    if (curTok().isNot(TokenKind::RParen)) {
      Diagnostic::emitError(curTok(), "Expected ')'");
      return invalidNode();
    }
    consumeToken(); // rparen
    return m_ASTRoot->registerNode<ast::NumberExpr>(
        ast::ASTDebugInfo{ sizeofTok },
        static_cast<ast::NumberExpr::ValueT>(type->getSizeof()),
        IntType::get());
  }

  auto expr{ parseParenExpr() };
  return m_ASTRoot->registerNode<ast::UnaryOpExpr>(
      ast::ASTDebugInfo{ sizeofTok }, std::move(expr),
      TokenKind::KW_sizeof);
}

auto Parser::parseUnaryOpExpr() -> Unique<ast::ExpressionNode> {
  if (!curTok().isOneOf(TokenKind::Plus, TokenKind::Minus, TokenKind::Amp,
                        TokenKind::Star, TokenKind::PlusPlus,
                        TokenKind::MinusMinus, TokenKind::Not)) {
    Diagnostic::emitError(curTok(),
                          "Expected unary operator in value expression");
    return invalidNode();
  }

  auto op{ curTok() };
  consumeToken();
  return m_ASTRoot->registerNode<ast::UnaryOpExpr>(
      ast::ASTDebugInfo{ op }, parseValueExpression(), op.getKind());
}

auto Parser::parseExpression() -> Ref<ast::ExpressionNode> {
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

auto Parser::parseBinOpRhs(Ref<ast::ExpressionNode> lhs,
                           int32_t prevPrecedence)
    -> Ref<ast::ExpressionNode> {
  while (true) {
    // precedence or -1 if not a binop
    auto tokPrecedence{ getTokPrecedence(curTok()) };
    if (tokPrecedence < prevPrecedence) {
      return lhs;
    }

    auto binOp{ curTok() };
    consumeToken();
    Ref<ast::ExpressionNode> rhs{ parseValueExpression() };
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

    // these are the same in IR
    if (binOp.is(TokenKind::AmpAmp)) {
      binOp.setKind(TokenKind::Amp);
    }
    if (binOp.is(TokenKind::PipePipe)) {
      binOp.setKind(TokenKind::Pipe);
    }

    auto getAssignTok{ [](TokenKind tok) -> std::optional<TokenKind> {
      switch (tok) {
      case TokenKind::PlusAssign:
        return TokenKind::Plus;
      case TokenKind::MinusAssign:
        return TokenKind::Minus;
      case TokenKind::StarAssign:
        return TokenKind::Star;
      case TokenKind::DivAssign:
        return TokenKind::Div;
      case TokenKind::AmpAssign:
        return TokenKind::Amp;
      case TokenKind::XorAssign:
        return TokenKind::Xor;
      case TokenKind::PipeAssign:
        return TokenKind::Div;
      default:
        return std::nullopt;
      }
    } };
    auto assignTok{ getAssignTok(binOp.getKind()) };

    if (assignTok) {
      auto actualOp{ m_ASTRoot->registerNode<ast::BinOpExpr>(
          ast::ASTDebugInfo{ binOp }, lhs, std::move(rhs), *assignTok) };
      lhs = m_ASTRoot->registerNode<ast::BinOpExpr>(
          ast::ASTDebugInfo{ binOp }, std::move(lhs), std::move(actualOp),
          TokenKind::Assign);
    } else {
      lhs = m_ASTRoot->registerNode<ast::BinOpExpr>(
          ast::ASTDebugInfo{ binOp }, std::move(lhs), std::move(rhs),
          binOp.getKind());
    }
  }
}

auto Parser::parseDeclspec() -> TypeRef {
  bort_assert(isTypenameStart(curTok()), "Expected type name");
  auto typeTok{ curTok() };
  consumeToken();
  Ref<Type> type;

  switch (typeTok.getKind()) {
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
    Diagnostic::emitError(typeTok, "Expected type name");
    return invalidNode();
  }

  while (curTok().is(TokenKind::Star)) {
    consumeToken();
    type = PointerType::get(type);
  }

  return type;
}

auto Parser::parseDeclarationStatement() -> Ref<ast::Statement> {
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

  auto varDecl{ parseVarDecl(type, nameTok) };

  if (curTok().isNot(TokenKind::Semicolon)) {
    Diagnostic::emitError(curTok(), "Expected ';'");
    return invalidNode();
  }
  consumeToken();

  return varDecl;
}

auto Parser::parseVarDecl(TypeRef type,
                          const Token& nameTok) -> Ref<ast::VarDecl> {
  std::string name{ nameTok.getStringView() };

  if (curTok().is(TokenKind::LBracket)) {
    consumeToken();
    if (curTok().isNot(TokenKind::NumericLiteral)) {
      Diagnostic::emitError(curTok(),
                            "Expected array size as numeric literal");
      return invalidNode();
    }

    auto size{ curTok().getLiteralValue<int32_t>() };
    consumeToken();

    if (curTok().isNot(TokenKind::RBracket)) {
      Diagnostic::emitError(curTok(), "Expected ']'");
      return invalidNode();
    }
    consumeToken();

    type = ArrayType::get(type, size);
  }

  auto node{ m_ASTRoot->registerNode<ast::VarDecl>(
      ast::ASTDebugInfo{ nameTok }, type, name) };

  if (curTok().is(TokenKind::Assign)) {
    consumeToken();
    if (curTok().isOneOf(TokenKind::LBrace, TokenKind::StringLiteral)) {
      node->setInitializer(parseInitializerList());
    } else {
      node->setInitializer(parseExpression());
    }
  }

  if (type->getKind() == TypeKind::Void) {
    Diagnostic::emitError(nameTok, "Variable of incomplete type 'void'");
    return invalidNode();
  }
  return node;
}

auto Parser::parseInitializerList() -> Unique<ast::InitializerList> {
  bort_assert(
      curTok().isOneOf(TokenKind::LBrace, TokenKind::StringLiteral),
      "Expected '{' or string literal");
  bool parseInitList{ curTok().is(TokenKind::LBrace) };
  auto startTok{ curTok() };
  consumeToken();
  std::vector<Ref<ast::NumberExpr>> values;

  if (parseInitList) {
    while (curTok().isNot(TokenKind::RBrace)) {
      Ref<ast::NumberExpr> value{ parseNumberExpr() };
      values.emplace_back(std::move(value));
      if (curTok().is(TokenKind::RBrace)) {
        continue;
      }

      if (curTok().isNot(TokenKind::Comma)) {
        Diagnostic::emitError(curTok(), "Expected ','");
        return invalidNode();
      }

      consumeToken(); // comma
    }
    consumeToken(); // rbrace
  } else {
    // it's string
    for (char c : startTok.getLiteralValue<std::string>()) {
      values.emplace_back(m_ASTRoot->registerNode<ast::NumberExpr>(
          ast::ASTDebugInfo{ startTok }, c, CharType::get()));
    }
    values.emplace_back(m_ASTRoot->registerNode<ast::NumberExpr>(
        ast::ASTDebugInfo{ startTok }, 0, CharType::get()));
  }

  if (values.empty()) {
    Diagnostic::emitError(startTok, "Initializer list can't be empty");
    return invalidNode();
  }

  return m_ASTRoot->registerNode<ast::InitializerList>(
      ast::ASTDebugInfo{ startTok }, std::move(values));
}

auto Parser::parseFunctionDecl(const TypeRef& type, const Token& nameTok)
    -> Ref<ast::FunctionDecl> {
  bort_assert(curTok().is(TokenKind::LParen), "Expected '('");
  consumeToken();

  std::string name{ nameTok.getStringView() };
  std::vector<Ref<Variable>> args;

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

    if (!curTok().isOneOf(TokenKind::Comma, TokenKind::RParen)) {
      Diagnostic::emitError(curTok(), "Expected ',' or ')'");
      return invalidNode();
    }
    // rparen will be consumed later on
    if (curTok().is(TokenKind::Comma)) {
      consumeToken();
    }

    args.push_back(makeRef<Variable>(
        std::string{ argNameTok.getStringView() }, argType));
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

auto Parser::parseFunctionCallExpr(const Token& nameTok)
    -> Unique<ast::FunctionCallExpr> {
  bort_assert(curTok().is(TokenKind::LParen), "Expected '('");
  consumeToken();

  std::string funcName{ nameTok.getStringView() };
  std::vector<Ref<ast::ExpressionNode>> args;
  while (curTok().isNot(TokenKind::RParen)) {
    args.emplace_back(parseExpression());

    if (curTok().is(TokenKind::RParen)) {
      break;
    }

    if (curTok().isNot(TokenKind::Comma)) {
      Diagnostic::emitError(curTok(), "Expected ','");
      return invalidNode();
    }
    consumeToken();
  }

  // consume RParen
  consumeToken();
  return m_ASTRoot->registerNode<ast::FunctionCallExpr>(
      ast::ASTDebugInfo{ nameTok }, makeRef<Function>(funcName),
      std::move(args));
}

auto Parser::parseIndexationExpr(const Token& nameTok)
    -> Unique<ast::ExpressionNode> {
  bort_assert(curTok().is(TokenKind::LBracket), "Expected '['");
  auto indexationStartTok{ curTok() };
  consumeToken();
  auto idxExpr{ parseExpression() };
  if (curTok().isNot(TokenKind::RBracket)) {
    Diagnostic::emitError(curTok(), "Expected ']'");
    return invalidNode();
  }
  consumeToken();

  auto varExpr{ m_ASTRoot->registerNode<ast::VariableExpr>(
      ast::ASTDebugInfo{ nameTok },
      makeRef<Variable>(std::string{ nameTok.getStringView() })) };

  return m_ASTRoot->registerNode<ast::IndexationExpr>(
      ast::ASTDebugInfo{ indexationStartTok }, std::move(varExpr),
      std::move(idxExpr));
}

auto Parser::parseStatement() -> Ref<ast::Statement> {
  if (curTok().is(TokenKind::LBrace)) {
    return parseBlock();
  }

  if (isTypenameStart(curTok())) {
    return parseDeclarationStatement();
  }

  switch (curTok().getKind()) {
  case TokenKind::KW_if:
    return parseIfStatement();
  case TokenKind::KW_while:
    return parseWhileStatement();
  case TokenKind::KW_return:
    return parseReturnStatement();
  case TokenKind::KW_for:
    return parseForStatement();
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

auto Parser::parseReturnStatement() -> Ref<ast::ReturnStmt> {
  bort_assert(curTok().is(TokenKind::KW_return), "Expected 'return'");

  auto kwTok{ curTok() };
  consumeToken();
  Ref<ast::ExpressionNode> expr{ nullptr };
  if (curTok().isNot(TokenKind::Semicolon)) {
    expr = parseExpression();
  }

  if (curTok().isNot(TokenKind::Semicolon)) {
    Diagnostic::emitError(curTok(), "Expected ';'");
    return invalidNode();
  }
  consumeToken();

  return m_ASTRoot->registerNode<ast::ReturnStmt>({ kwTok }, expr);
}

/// desugared into
/// {
///   init;
///   while (condition) {
///     ...
///     update;
///   }
auto Parser::parseForStatement() -> Ref<ast::Block> {
  bort_assert(curTok().is(TokenKind::KW_for), "Expected 'for'");
  auto forTok{ curTok() };
  consumeToken();

  if (curTok().isNot(TokenKind::LParen)) {
    Diagnostic::emitError(curTok(), "Expected '('");
    return invalidNode();
  }
  consumeToken();

  Ref<ast::VarDecl> init;
  if (curTok().isNot(TokenKind::Semicolon)) {
    // a little trick
    init = dynCastRef<ast::VarDecl>(parseDeclarationStatement());
    if (!init) {
      Diagnostic::emitError(curTok(), "Expected variable declaration");
      return invalidNode();
    }
  }

  Ref<ast::ExpressionNode> condition;
  if (curTok().isNot(TokenKind::Semicolon)) {
    condition = parseExpression();
    if (curTok().isNot(TokenKind::Semicolon)) {
      Diagnostic::emitError(curTok(), "Expected ';'");
      return invalidNode();
    }
    consumeToken();
  }

  if (!condition) {
    condition = m_ASTRoot->registerNode<ast::NumberExpr>(
        ast::ASTDebugInfo{ curTok() }, 1, IntType::get());
  }

  Ref<ast::ExpressionNode> update;
  auto updateTok{ curTok() };
  if (curTok().isNot(TokenKind::Semicolon)) {
    update = parseExpression();
  }

  if (curTok().isNot(TokenKind::RParen)) {
    Diagnostic::emitError(curTok(), "Expected ')'");
    return invalidNode();
  }
  consumeToken();

  if (curTok().isNot(TokenKind::LBrace)) {
    Diagnostic::emitError(curTok(), "Expected '{{'");
    return invalidNode();
  }
  auto body{ parseBlock() };

  if (update) {
    body->pushChild(m_ASTRoot->registerNode<ast::ExpressionStmt>(
        ast::ASTDebugInfo{ updateTok }, std::move(update)));
  }

  auto outerBlock{ m_ASTRoot->registerNode<ast::Block>(
      ast::ASTDebugInfo{ curTok() }) };
  outerBlock->pushChild(init);
  outerBlock->pushChild(m_ASTRoot->registerNode<ast::WhileStmt>(
      ast::ASTDebugInfo{ forTok }, std::move(condition),
      std::move(body)));

  return outerBlock;
}

} // namespace bort
