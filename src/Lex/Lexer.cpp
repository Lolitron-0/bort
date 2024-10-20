#include "bort/Lex/Lexer.hpp"
#include "bort/CLI/IO.hpp"
#include "bort/Frontend/SourceFile.hpp"
#include "bort/Lex/Token.hpp"
#include <algorithm>
#include <cctype>
#include <cul/cul.hpp>
#include <memory>
#include <string_view>

namespace bort {

static auto startsWith(const SourceFileIt& pos,
                       std::string_view prefix) -> bool {
  // TODO: check overflow
  return std::equal(prefix.begin(), prefix.end(), pos.asBufIter());
}

// clang-format off
#undef TOK
#undef KEYWORD
#define KEYWORD(t) .Case(#t, TokenKind::KW_ ## t)
static constexpr cul::BiMap s_IdentifierMapping{
  [](auto selector) {
    return selector 
#include "bort/Lex/Tokens.def"
      ;
  }
};
#undef KEYWORD
// clang-format on

static auto lexIdentifier(SourceFileIt& pos) -> bool {
  auto start{ pos };
  size_t length{ 1 };
  // TODO: actually there is a more sophisticated logic in acceptable
  // identifier characters, but we'll leave it for now
  while (std::isalnum(*pos) || *pos == '_') {
    ++pos;
    ++length;
  }
  if (length == 0) {
    return false;
  }

  Token newTok{ TokenKind::Unknown, start, length };
  auto kind{ s_IdentifierMapping.FindByFirst(newTok.getValue()) };
  newTok.setKind(
      kind.value_or(TokenKind::Identifier)); // either keyword or name
  return true;
}

void Lexer::lex(const std::shared_ptr<SourceFile>& file) {
  SourceFileIt pos{ file };

  while (pos) {

    // Line comments
    if (startsWith(pos, "//")) {
      pos += 2;
      while (*pos != '\n') {
        ++pos;
      }
      continue;
    }

    // Block comments
    if (startsWith(pos, "/*")) {
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
    if (std::isdigit(*pos)) {
      auto start{ pos };
      size_t length{ 1 };
      // we only supprot decimal integers for now
      while (std::isdigit(*pos)) {
        ++pos;
        ++length;
      }
      m_Tokens.emplace_back(TokenKind::NumericConstant, start, length);
      continue;
    }

    // Identifier

    DEBUG_OUT("Unknown tok: {}", *pos);
    pos++;
  }
}

} // namespace bort
