#include "bort/Lex/Lexer.hpp"
#include "bort/CLI/IO.hpp"
#include "bort/Frontend/SourceFile.hpp"
#include <algorithm>
#include <iterator>
#include <string_view>

namespace bort {

static auto startsWith(SourceFileIt pos,
                       std::string_view prefix) -> bool {
  // TODO: check overflow
  return std::equal(prefix.begin(), prefix.end(), pos.asBufIter());
}

void Lexer::lex(SourceFile& file) {
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
      auto blockCommentEnd{ file.getBuffer().find_first_of(
          "*/", pos.getIndex()) };
      if (blockCommentEnd == std::string::npos) {
        emitError(pos, 2, "Unterminated block comment");
      }
      pos += (blockCommentEnd - pos.getIndex() + 2);
      continue;
    }

    DEBUG_OUT("Unknown tok: {}", *pos);
    pos++;
  }
}

} // namespace bort
