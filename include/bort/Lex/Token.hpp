#pragma once
#include "bort/Frontend/SourceFile.hpp"
#include <string_view>

namespace bort {

enum class TokenKind {
#define TOK(t) t, // NOLINT
#include "bort/Lex/Token.def"
  NumTokens
};

class Token {
public:
  Token(TokenKind kind, SourceFileIt loc);

  [[nodiscard]] auto is(TokenKind kind) const -> bool {
    return m_Kind == kind;
  }
  [[nodiscard]] auto isOneOf(TokenKind tk1, TokenKind tk2) const -> bool {
    return m_Kind == tk1 || m_Kind == tk2;
  }
  template <typename... TKs>
  [[nodiscard]] auto isOneOf(TokenKind tk, TKs... other) const -> bool {
    return is(tk) || isOneOf(other...);
  }

  void setKind(TokenKind kind) {
    m_Kind = kind;
  }

  [[nodiscard]] auto getLoc() const -> SourceFileIt {
    return m_Loc;
  }

  [[nodiscard]] auto getValue() const -> std::string_view {
    return std::string_view{
      m_Loc.asBufIter(),
      m_Loc.asBufIter() +
          static_cast<SourceFileIt::difference_type>(m_Length)
    };
  }

private:
  TokenKind m_Kind;
  SourceFileIt m_Loc;
  size_t m_Length;
};

} // namespace bort
