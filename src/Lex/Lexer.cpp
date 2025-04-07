#include "bort/Lex/Lexer.hpp"
#include "bort/CLI/IO.hpp"
#include "bort/Frontend/SourceFile.hpp"
#include "bort/Lex/Token.hpp"
#include "bort/Lex/Utils.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <cul/cul.hpp>
#include <iterator>
#include <memory>
#include <string_view>
#include <vector>

namespace bort {

// clang-format off
#undef TOK
#undef KEYWORD
#undef PPTOK
#define KEYWORD(t) .Case(#t, TokenKind::KW_ ## t)
#define PPTOK(t) .Case("#" #t, TokenKind::PP_ ## t)
static constexpr cul::BiMap s_IdentifierMapping{
  [](auto selector) {
    return selector 
#include "bort/Lex/Tokens.def"
      ;
  }
};
#undef KEYWORD
#undef PPTOK
// clang-format on

// clang-format off
#undef TOK
#undef PUNCT
#define PUNCT(t, s) .Case(s, TokenKind::t)
static constexpr cul::BiMap s_PunctuatorMapping{
  [](auto selector) {
    return selector 
#include "bort/Lex/Tokens.def" 
      ;
  }
};
// clang-format on

#undef PUNCT
#define PUNCT(t, s) t,
namespace detail {
enum class CountPunctuators {
#include "bort/Lex/Tokens.def" // LParen, RParen, Equals, ...
  value
};
constexpr size_t CountPunctuators_v =
    static_cast<size_t>(CountPunctuators::value);
} // namespace detail

#undef PUNCT
#define PUNCT(t, s) s,
static constexpr std::array<std::string_view, detail::CountPunctuators_v>
    s_PunctuatorStrings = sortConstexpr(
        std::array<std::string_view, detail::CountPunctuators_v>{
#include "bort/Lex/Tokens.def" // "<=", "<", "&&", ...
        },
        [](const auto& a, const auto& b) {
          return a.length() > b.length();
        });
#undef PUNCT

static auto decodeEscapedChar(const SourceFileIt& pos) -> char {
  switch (*pos) {
  case 'a':
    return '\a';
  case 'b':
    return '\b';
  case 'f':
    return '\f';
  case 'n':
    return '\n';
  case 'r':
    return '\r';
  case 't':
    return '\t';
  case 'v':
    return '\v';
  case '0':
    return '\0';
  case '\\':
    return '\\';
  default:
    Diagnostic::emitWarning(pos, 1,
                            "Invalid escape sequence, ignoring '\\'");
    return *pos;
  }
}

auto Lexer::lexIdentifier(SourceFileIt& pos) -> bool {
  auto start{ pos };
  auto ident{ consumeIdent(pos) };

  if (ident.empty()) {
    return false;
  }

  Token newTok{ TokenKind::Unknown, start, ident.size() };
  auto kind{ s_IdentifierMapping.FindByFirst(newTok.getStringView()) };
  newTok.setKind(
      kind.value_or(TokenKind::Identifier)); // either keyword or name
  m_Tokens->push_back(std::move(newTok));
  return true;
}

auto Lexer::lexNumericLiteral(SourceFileIt& pos) -> bool {
  auto start{ pos };
  if (!std::isdigit(*pos)) {
    return false;
  }

  size_t length{ 1 };
  ++pos;

  // we only support decimal integers for now
  while (std::isdigit(*pos)) {
    ++pos;
    ++length;
  }
  m_Tokens->emplace_back(TokenKind::NumericLiteral, start, length);
  m_Tokens->back().setLiteralValue(
      std::stoi(std::string{ start.getValue(length) }));
  return true;
}

auto Lexer::lexStringLiteral(SourceFileIt& pos) -> bool {
  auto start{ pos };
  if (*pos != '"') {
    return false;
  }

  ++pos;
  std::string value{};

  while (*pos != '"' && *pos != '\n') {
    if (*pos == '\\') {
      ++pos; // consume backslash
      value += decodeEscapedChar(pos);
      ++pos;
      continue;
    }
    value += *pos;
    ++pos;
  }
  if (*pos == '"') {
    ++pos;
  } else {
    Diagnostic::emitError(start, value.length(),
                          "Missing terminating \"");
    throw LexerFatalError();
    return false;
  }
  m_Tokens->emplace_back(TokenKind::StringLiteral, start,
                         value.length() + 2); // plus quotes
  m_Tokens->back().setLiteralValue(value);
  return true;
}

auto Lexer::lexCharLiteral(SourceFileIt& pos) -> bool {
  auto start{ pos };
  if (*pos != '\'') {
    return false;
  }
  ++pos;            // consume first quote
  auto length{ 3 }; // normal char literal expr length

  char value{ *pos };
  if (*pos == '\\') {
    ++pos; // consume backslash
    ++length;
    value = decodeEscapedChar(pos);
  }
  ++pos; // consume actual char

  if (*pos != '\'') {
    Diagnostic::emitError(start, length, "Missing terminating '");
    throw LexerFatalError();
    return false;
  }

  ++pos; // consume last quote
  m_Tokens->emplace_back(TokenKind::CharLiteral, start, length);
  m_Tokens->back().setLiteralValue(value);
  return true;
}

auto Lexer::lexPunctuator(SourceFileIt& pos) -> bool {
  // s_PunctuatorStrings is sorted by length descending, so we'll match
  // long punctuators first
  for (const auto& punct : s_PunctuatorStrings) {
    if (startsWith(pos, punct)) {
      auto pucntKindOpt{ s_PunctuatorMapping.FindByFirst(punct) };
      bort_assert(pucntKindOpt.has_value(),
                  "Punctuator mapping and list somehow doesn't match");
      m_Tokens->emplace_back(pucntKindOpt.value(), pos, punct.length());
      pos += punct.length();
      return true;
    }
  }

  return false;
}

Lexer::Lexer() {
  m_Tokens = std::make_shared<TokenList>();
}

void Lexer::lex(const std::shared_ptr<SourceFile>& file) {
  SourceFileIt pos{ file };

  while (pos) {

    // Line comments
    if (startsWith(pos, "//")) {
      auto commentStart{ pos };
      pos += 2;
      while (*pos != '\n') {
        ++pos;
      }
    }

    // Block comments
    if (startsWith(pos, "/*")) {
      auto blockCommentStart{ pos };
      pos += 2;
      auto blockCommentEnd{ file->getBuffer().find_first_of(
          "*/", pos.getIndex()) };
      if (blockCommentEnd == std::string::npos) {
        Diagnostic::emitError(pos, 2, "Unterminated block comment");
      }
      pos += (blockCommentEnd - pos.getIndex() + 2);
      continue;
    }

    // Newlines and spaces
    if (*pos == '\n' || *pos == ' ') {
      ++pos;
      continue;
    }

    // Numeric literals
    if (lexNumericLiteral(pos)) {
      continue;
    }

    // String literals
    if (lexStringLiteral(pos)) {
      continue;
    }

    // Char literals
    if (lexCharLiteral(pos)) {
      continue;
    }

    // Identifiers
    if (lexIdentifier(pos)) {
      continue;
    }

    // Punctuators
    if (lexPunctuator(pos)) {
      continue;
    }

    Diagnostic::emitError(pos, 1, "Unknown token");
    throw LexerFatalError();
  }

  m_Tokens->emplace_back(TokenKind::Eof, pos, 0);
}

auto Lexer::getTokens() const -> std::shared_ptr<TokenList> {
  return m_Tokens;
}

LexerFatalError::LexerFatalError()
    : FrontEndFatalError{ "Lexer fatal failure. Aborting." } {
}

} // namespace bort
