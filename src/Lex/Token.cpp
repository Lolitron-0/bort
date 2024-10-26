#include <cassert>
#include <utility>

#include "bort/Lex/Token.hpp"

namespace bort {

Token::Token(TokenKind kind, SourceFileIt loc, size_t length)
    : m_Kind{ kind },
      m_Loc{ std::move(loc) },
      m_Length{ length } {
}

TokenKind Token::getKind() const {
  return m_Kind;
}

void Token::setKind(TokenKind kind) {
  m_Kind = kind;
}

auto Token::getLoc() const -> SourceFileIt {
  return m_Loc;
}

auto Token::getString() const -> std::string_view {
  assert(!is(TokenKind::Eof) && "Get string on EOF token");
  return m_Loc.getValue(m_Length);
}

auto Token::getLength() const -> size_t {
  return m_Length;
}

} // namespace bort
