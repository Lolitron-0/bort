#pragma once
#include "bort/AST/ASTNode.hpp"
#include "bort/AST/BreakStmt.hpp"
#include "bort/AST/ContinueStmt.hpp"
#include "bort/AST/ExpressionNode.hpp"
#include "bort/AST/FunctionCallExpr.hpp"
#include "bort/AST/FunctionDecl.hpp"
#include "bort/AST/GotoStmt.hpp"
#include "bort/AST/IfStmt.hpp"
#include "bort/AST/InitializerList.hpp"
#include "bort/AST/LabelStmt.hpp"
#include "bort/AST/NumberExpr.hpp"
#include "bort/AST/ReturnStmt.hpp"
#include "bort/AST/VarDecl.hpp"
#include "bort/AST/WhileStmt.hpp"
#include "bort/Basic/Ref.hpp"
#include "bort/Frontend/Type.hpp"
#include "bort/Lex/Lexer.hpp"
#include <cstddef>
#include <optional>

namespace bort {

/// @todo don't check null on error and then traverse, checking for
/// null, to recieve more errors at once
class Parser {
public:
  explicit Parser(const Ref<TokenList>& tokenList)
      : m_Tokens{ tokenList },
        m_CurTokIter{ tokenList->begin() },
        m_ASTRoot{ makeUnique<ast::ASTRoot>() } {
  }

  /// top-level \n
  /// -> globalVarDecl \n
  /// -> functionDecl \n
  /// (for testing can be anything)
  ///
  /// parses all top-level statements and populates AST root with them
  auto buildAST() -> Ref<ast::ASTRoot>;

  [[nodiscard]] auto isASTInvalid() const -> bool {
    return m_ASTInvalid;
  }

  // for docs
protected:
  /// number -> [0-9]+ | charLiteral
  auto parseNumberExpr() -> Unique<ast::NumberExpr>;
  /// parenExpr \n
  /// -> '(' expression ')' \n
  /// @todo type-casts -> '(' declspec ')'
  auto parseParenExpr() -> Ref<ast::ExpressionNode>;
  /// identifier \n
  /// -> varExpr \n
  /// -> functionCallExpr \n
  /// -> identifier indexationExpr
  auto parseIdentifierExpr() -> Unique<ast::ExpressionNode>;
  /// value expression \n
  /// -> number \n
  /// -> parenExpr \n
  /// -> sizeofExpr \n
  /// -> unaryOpExpr \n
  /// -> lvalue
  auto parseValueExpression() -> Ref<ast::ExpressionNode>;
  /// sizeofExpr -> 'sizeof' (parenExpr | '(' declspec ')' )
  auto parseSizeofExpr() -> Unique<ast::ExpressionNode>;
  /// varExpr -> identifier
  auto parseVarExpr() -> Unique<ast::ExpressionNode>;
  /// unaryOpExpr -> unaryOp valueExpression
  auto parseUnaryOpExpr() -> Unique<ast::ExpressionNode>;
  /// expression
  /// -> valueExpression binOpRhs
  auto parseExpression() -> Ref<ast::ExpressionNode>;
  /// binOpRhs
  /// -> bipOp valueExpression (binOpRhs ...)
  auto parseBinOpRhs(Ref<ast::ExpressionNode> lhs,
                     int32_t prevPrecedence = 0)
      -> Ref<ast::ExpressionNode>;
  /// declspec -> ( 'int' | 'void' | 'char' ) ('*'...)
  /// @todo type qualifiers
  auto parseDeclspec() -> TypeRef;
  /// declaration -> declspec (varDecl ';' |  functionDecl)
  auto parseDeclarationStatement() -> Ref<ast::Statement>;
  /// varDecl -> declspec identifier initializerExpr?
  /// @todo declspec (identifier ('=' expr)?, ...)
  auto parseVarDecl(TypeRef type,
                    const Token& nameTok) -> Ref<ast::VarDecl>;
  /// initializerList -> '{' number, ... '}' | stringLiteral
  auto parseInitializerList() -> Unique<ast::InitializerList>;
  /// functionDecl -> identifier '(' (declspec ident, ...) ')' block
  auto parseFunctionDecl(const TypeRef& type,
                         const Token& nameTok) -> Ref<ast::FunctionDecl>;
  /// functionCallExpr -> nameTok '(' (declspec identifier, ...) ')'
  auto parseFunctionCallExpr(const Token& nameTok)
      -> Unique<ast::FunctionCallExpr>;
  /// indexationExpr -> nameTok '[' expr ']'
  auto parseIndexationExpr(const Token& nameTok)
      -> Unique<ast::ExpressionNode>;
  /// statement \n
  /// -> expression ';' \n
  /// -> declarationStatement ';' \n
  /// -> block \n
  /// -> ifStatement \n
  /// -> whileStatement \n
  /// -> returnStatement \n
  /// -> breakStatement \n
  /// -> continueStatement \n
  /// -> labelStatement
  auto parseStatement() -> Ref<ast::Statement>;
  /// breakStatement -> 'break' ';'
  auto parseBreakStatement() -> Ref<ast::BreakStmt>;
  /// continueStatement -> 'continue' ';'
  auto parseContinueStatement() -> Ref<ast::ContinueStmt>;
  /// block
  /// -> '{' statement... '}'
  auto parseBlock() -> Unique<ast::Block>;
  /// ifStatement -> 'if' parenExpr block (else block)?
  auto parseIfStatement() -> Ref<ast::IfStmt>;
  /// whileStatement -> 'while' parenExpr block
  auto parseWhileStatement() -> Ref<ast::WhileStmt>;
  /// forStatement -> 'for' '(' varDecl ';' expr? ';' expr? ')' block
  auto parseForStatement() -> Ref<ast::Block>;
  /// returnStatement -> 'return' (expr)? ';'
  auto parseReturnStatement() -> Ref<ast::ReturnStmt>;
  /// labelStatement -> identifier ':' ';'
  auto parseLabelStatement() -> Ref<ast::LabelStmt>;
  /// gotoStatement -> 'goto' identifier ';'
  auto parseGotoStatement() -> Ref<ast::GotoStmt>;

private:
  void disableDiagnostics() {
    m_DiagnosticSilenced = true;
  }
  void enableDiagnostics() {
    m_DiagnosticSilenced = false;
  }

  [[nodiscard]] auto isFunctionDecl() const -> bool;

  [[nodiscard]] auto lookahead(uint32_t offset) const -> const Token&;

  [[nodiscard]] inline auto curTok() const -> const Token& {
    return *m_CurTokIter;
  }

  auto invalidNode() -> std::nullptr_t;

  inline void consumeToken() {
    m_CurTokIter++;
  }

  Ref<TokenList> m_Tokens;
  TokenList::const_iterator m_CurTokIter;
  Ref<ast::ASTRoot> m_ASTRoot;
  bool m_ASTInvalid{ false };
  bool m_DiagnosticSilenced{ false };
  bool m_InsideLoop{ false };
  bool m_GlobalScope{ true };
};

} // namespace bort
