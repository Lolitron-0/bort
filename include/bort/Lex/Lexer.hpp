#pragma once
#include "bort/Frontend/SourceFile.hpp"
#include "bort/Lex/Token.hpp"
#include <list>

namespace bort {

using TokenList = std::list<Token>;

class Lexer {
public:
  void lex(const std::shared_ptr<SourceFile>& file);

private:
  auto lexNumericLiteral(SourceFileIt& pos) -> bool;
  auto lexIdentifier(SourceFileIt& pos) -> bool;
  auto lexStringLiteral(SourceFileIt& pos) -> bool;
  auto lexCharLiteral(SourceFileIt& pos) -> bool;

  TokenList m_Tokens;
};

} // namespace bort
