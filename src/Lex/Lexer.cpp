#include "bort/Lex/Lexer.hpp"
#include "bort/CLI/IO.hpp"
#include "bort/Frontend/SourceFile.hpp"
#include "bort/Lex/Token.hpp"
#include "bort/Lex/Utils.hpp"
#include <cctype>
#include <cul/cul.hpp>
#include <memory>
#include <string_view>

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

static char decodeEscapedChar(const SourceFileIt& pos) {
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
  default:
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
  auto kind{ s_IdentifierMapping.FindByFirst(newTok.getString()) };
  newTok.setKind(
      kind.value_or(TokenKind::Identifier)); // either keyword or name
  m_Tokens.push_back(std::move(newTok));
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
  m_Tokens.emplace_back(TokenKind::NumericConstant, start, length);
  m_Tokens.back().setLiteralValue(
      std::stoi(std::string{ start.getValue(length) }));
  return true;
}

auto Lexer::lexStringLiteral(SourceFileIt& pos) -> bool {
  auto start{ pos };
  if (*pos != '"') {
    return false;
  }

  auto length{ 1 };
  ++pos;

  while (*pos != '"' && *pos != '\n') {
    ++pos;
    ++length;
  }
  if (*pos == '"') {
    ++pos;
    ++length;
  } else {
    emitError(start, length, "Unclosed string literal");
    return false;
  }
  m_Tokens.emplace_back(TokenKind::StringLiteral, start, length);
  // get unquoted value
  m_Tokens.back().setLiteralValue(
      std::string{ (++start).getValue(length - 2) });
  return true;
}

auto Lexer::lexCharLiteral(SourceFileIt& pos) -> bool {
  auto start{ pos };
  if (*pos != '\'') {
    return false;
  }
  ++pos;
  auto length{ 3 }; // normal char literal expr length

  char value{ *pos };
  if (*pos == '\\') {
    ++pos;
    ++length;
    value = decodeEscapedChar(pos);
    if (value == *pos) {
      emitError(start, 4, "Invalid escape sequence");
      return false; // TODO: throw fatal
    }
  }
  ++pos;

  if (*pos != '\'') {
    emitError(start, length, "Unclosed character literal");
    return false;
  }

  m_Tokens.emplace_back(TokenKind::CharConstant, start, length);
  m_Tokens.back().setLiteralValue(value);
  return true;
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
        emitError(pos, 2, "Unterminated block comment");
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

    DEBUG_OUT("Unknown tok: {}", *pos);
    pos++;
  }
}

} // namespace bort
