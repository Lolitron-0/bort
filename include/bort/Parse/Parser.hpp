#pragma once
#include "bort/AST/ASTNode.hpp"
#include "bort/AST/ExpressionNode.hpp"
#include "bort/Basic/Ref.hpp"
#include "bort/Frontend/Type.hpp"
#include "bort/Lex/Lexer.hpp"

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

  // for docs
protected:
  /// number -> {integer}
  auto parseNumberExpr() -> Unique<ast::NumberExpr>;
  /// parenExpr -> '(' expression ')'
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
  auto parseDeclspec() -> Ref<Type>;
  /// vardecl -> declspec identifier ';'
  /// @todo declspec (identifier ('=' expr)?, ...) ';'
  auto parseVarDecl(const TypeRef& type) -> Ref<ast::VarDecl>;
  /// block
  /// -> '{' statement... '}'
  auto parseBlock() -> Unique<ast::Block>;

private:
  [[nodiscard]] inline auto curTok() -> const Token& {
    return *m_CurTokIter;
  }

  inline void consumeToken() {
    m_CurTokIter++;
  }

  Ref<TokenList> m_Tokens;
  TokenList::const_iterator m_CurTokIter;
  Ref<ast::ASTRoot> m_ASTRoot;
};

} // namespace bort
