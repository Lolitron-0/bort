#pragma once
#include "bort/Frontend/SourceFile.hpp"
#include "bort/Lex/Token.hpp"
#include <list>

namespace bort {

using TokenList = std::list<Token>;

class Lexer {
public:
  void lex(SourceFile& file);

private:
  TokenList m_Tokens;
};

} // namespace bort
