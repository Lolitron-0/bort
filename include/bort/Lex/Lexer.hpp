#pragma once
#include "bort/Frontend/FrontendInstance.hpp"
#include "bort/Frontend/SourceFile.hpp"
#include "bort/Lex/Token.hpp"
#include <list>
#include <memory>

namespace bort {

using TokenList = std::list<Token>;

class LexerFatalError : public FrontendFatalError {
public:
  LexerFatalError();
};

class Lexer {
public:
  Lexer();
  void lex(const std::shared_ptr<SourceFile>& file);

  [[nodiscard]] auto getTokens() const -> std::shared_ptr<TokenList>;

private:
  /// @todo return optional token and make non-member
  auto lexNumericLiteral(SourceFileIt& pos) -> bool;
  auto lexIdentifier(SourceFileIt& pos) -> bool;
  auto lexStringLiteral(SourceFileIt& pos) -> bool;
  auto lexCharLiteral(SourceFileIt& pos) -> bool;
  auto lexPunctuator(SourceFileIt& pos) -> bool;

  std::shared_ptr<TokenList> m_Tokens;
};

} // namespace bort
