#include <utility>

#include "bort/Basic/Assert.hpp"
#include "bort/Lex/Token.hpp"

namespace bort {

Token::Token(TokenKind kind, SourceFileIt loc, size_t length)
    : m_Kind{ kind },
      m_Loc{ std::move(loc) },
      m_Length{ length } {
}

auto Token::getKind() const -> TokenKind {
  return m_Kind;
}

void Token::setKind(TokenKind kind) {
  m_Kind = kind;
}

auto Token::getLoc() const -> SourceFileIt {
  return m_Loc;
}

auto Token::getStringView() const -> std::string_view {
  bort_assert(!is(TokenKind::Eof), "EOF token has no value");
  return m_Loc.getValue(m_Length);
}

auto Token::getLength() const -> size_t {
  return m_Length;
}

} // namespace bort
