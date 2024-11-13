#pragma once
#include "bort/AST/ASTNode.hpp"
#include "bort/AST/ExpressionNode.hpp"
#include "bort/AST/FunctionDecl.hpp"
#include "bort/Basic/Ref.hpp"
#include "bort/Frontend/Type.hpp"
#include "bort/Lex/Lexer.hpp"
#include <cstddef>

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
  /// number -> {integer}
  auto parseNumberExpr() -> Unique<ast::NumberExpr>;
  /// parenExpr \n
  /// -> '(' expression ')' \n
  /// @todo type-casts -> '(' declspec ')'
  auto parseParenExpr() -> Unique<ast::ExpressionNode>;
  /// identifier \n
  /// -> identifier - variable \n
  /// -> identifier '(' expr, ... ')' - function call
  auto parseIdentifierExpr() -> Unique<ast::ExpressionNode>;
  /// value expression \n
  /// -> number \n
  /// -> parenExpr \n
  /// -> identifierExpr
  auto parseValueExpression() -> Unique<ast::ExpressionNode>;
  /// expression
  /// -> valueExpression (binOp valueExpression ...)
  auto parseExpression() -> Unique<ast::ExpressionNode>;
  /// binOpRhs
  /// -> bipOp valueExpression (binOpRhs ...)
  auto parseBinOpRhs(Unique<ast::ExpressionNode> lhs,
                     int32_t prevPrecedence = 0)
      -> Unique<ast::ExpressionNode>;
  /// declspec -> ( 'int' | 'void' | 'char' ) ('*'...)
  /// @todo type qualifiers
  auto parseDeclspec() -> TypeRef;
  /// declaration -> declspec (varDecl |  functionDecl)
  auto parseDeclaration() -> Ref<ast::Statement>;
  /// varDecl -> identifier ';'
  /// @todo declspec (identifier ('=' expr)?, ...) ';'
  auto parseVarDecl(const TypeRef& type,
                    const Token& nameTok) -> Ref<ast::VarDecl>;
  /// functionDecl -> identifier '(' (declspec ident, ...) ')' block
  auto parseFunctionDecl(const TypeRef& type,
                         const Token& nameTok) -> Ref<ast::FunctionDecl>;
  /// statement \n
  /// -> expression ';' \n
  /// -> block \n
  auto parseStatement() -> Ref<ast::Statement>;
  /// block
  /// -> '{' statement... '}'
  auto parseBlock() -> Unique<ast::Block>;

private:
  [[nodiscard]] auto isFunctionDecl() const -> bool;

  [[nodiscard]] auto lookahead(uint32_t offset) const -> const Token&;

  [[nodiscard]] inline auto curTok() const -> const Token& {
    return *m_CurTokIter;
  }

  [[nodiscard]] auto invalidNode() -> std::nullptr_t {
    m_ASTInvalid = true;
    return nullptr;
  }

  inline void consumeToken() {
    m_CurTokIter++;
  }

  Ref<TokenList> m_Tokens;
  TokenList::const_iterator m_CurTokIter;
  Ref<ast::ASTRoot> m_ASTRoot;
  bool m_ASTInvalid{ false };
};

} // namespace bort
