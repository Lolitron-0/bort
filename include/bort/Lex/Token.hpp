#pragma once
#include "bort/Frontend/SourceLocation.hpp"

namespace bort {

enum class TokenKind {
#define TOK(t) t, // NOLINT
#include "bort/Lex/Token.def"
  NumTokens
};

class Token {
public:
  Token(TokenKind kind, SourceLocation loc);

  [[nodiscard]] auto Is(TokenKind kind) const -> bool {
    return Kind == kind;
  }
  [[nodiscard]] auto IsOneOf(TokenKind tk1, TokenKind tk2) const -> bool {
    return Kind == tk1 || Kind == tk2;
  }
  template <typename... TKs>
  [[nodiscard]] auto IsOneOf(TokenKind tk, TKs... other) const -> bool {
    return Is(tk) || IsOneOf(other...);
  }

  void SetKind(TokenKind kind);

  [[nodiscard]] auto GetLoc() const -> SourceLocation;
  void SetLoc(const SourceLocation& loc) {
    Loc = loc;
  }

private:
  TokenKind Kind;
  SourceLocation Loc;
};

} // namespace bort
