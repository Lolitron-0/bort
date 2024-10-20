#include "bort/Lex/Token.hpp"

namespace bort {

Token::Token(TokenKind kind, SourceFileIt loc, size_t length)
    : m_Kind{ kind },
      m_Loc{ loc },
      m_Length{ length } {
}

void Token::setKind(TokenKind kind) {
  m_Kind = kind;
}

auto Token::getLoc() const -> SourceFileIt {
  return m_Loc;
}

auto Token::getValue() const -> std::string_view {
  return std::string_view{ m_Loc.asBufIter(),
                           m_Loc.asBufIter() +
                               static_cast<SourceFileIt::difference_type>(
                                   m_Length) };
}

} // namespace bort
