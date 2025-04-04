#pragma once
#include "bort/Basic/Assert.hpp"
#include "bort/Frontend/SourceFile.hpp"
#include "cul/BiMap.hpp"
#include <string_view>
#include <type_traits>
#include <variant>

namespace bort {

enum class TokenKind {
#define TOK(t) t, // NOLINT
#include "bort/Lex/Tokens.def"
  NUM_TOKENS
};

class Token {
public:
  using LiteralValue =
      std::variant<std::monostate, std::string, int, char>;

  Token(TokenKind kind, SourceFileIt loc, size_t length);

  [[nodiscard]] inline auto is(TokenKind kind) const -> bool {
    return m_Kind == kind;
  }
  [[nodiscard]] inline auto isNot(TokenKind kind) const -> bool {
    return !is(kind);
  }
  [[nodiscard]] constexpr auto isOneOf(TokenKind tk1,
                                       TokenKind tk2) const -> bool {
    return m_Kind == tk1 || m_Kind == tk2;
  }
  template <typename... TKs>
  [[nodiscard]] constexpr auto isOneOf(TokenKind tk,
                                       TKs... other) const -> bool {
    return is(tk) || isOneOf(other...);
  }

  [[nodiscard]] auto getKind() const -> TokenKind;
  void setKind(TokenKind kind);

  [[nodiscard]] auto getLoc() const -> SourceFileIt;

  [[nodiscard]] auto getStringView() const -> std::string_view;
  [[nodiscard]] auto getLength() const -> size_t;

  template <typename T>
  void setLiteralValue(T&& value) {
    m_LiteralValue.emplace<std::decay_t<T>>(std::forward<T>(value));
  }

  template <typename Func>
  auto visitLiteralValue(Func&& func) const {
    return std::visit(std::forward<Func>(func), m_LiteralValue);
  }

  template <typename T>
  auto getLiteralValue() const -> T {
    auto result{ std::get<T>(m_LiteralValue) };
    return result;
  }

  static auto getTokenName(TokenKind kind) -> std::string_view;

  // clang-format off
#undef TOK
#define TOK(t) .Case(#t, TokenKind::t)
  static constexpr cul::BiMap TokenNameMapping{
    [](auto selector) {
      return selector 
#include "bort/Lex/Tokens.def"
        ;
    }
  };
  // clang-format on

private:
  TokenKind m_Kind;
  SourceFileIt m_Loc;
  size_t m_Length;
  LiteralValue m_LiteralValue{ std::monostate{} };
};

} // namespace bort
