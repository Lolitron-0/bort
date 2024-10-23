#pragma once
#include "bort/Frontend/FrontendInstance.hpp"
#include "bort/Frontend/SourceFile.hpp"
#include "bort/Lex/Token.hpp"
#include <list>

namespace bort {

using TokenList = std::list<Token>;

class LexerFatalError : public FrontendFatalError {
public:
  LexerFatalError();
};

class Lexer {
public:
  void lex(const std::shared_ptr<SourceFile>& file);

private:
  auto lexNumericLiteral(SourceFileIt& pos) -> bool;
  auto lexIdentifier(SourceFileIt& pos) -> bool;
  auto lexStringLiteral(SourceFileIt& pos) -> bool;
  auto lexCharLiteral(SourceFileIt& pos) -> bool;
  auto lexPunctuator(SourceFileIt& pos) -> bool;

  TokenList m_Tokens;
};

} // namespace bort
