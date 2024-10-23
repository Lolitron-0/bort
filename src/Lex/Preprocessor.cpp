#include "bort/Lex/Preprocessor.hpp"
#include "bort/CLI/IO.hpp"
#include "bort/Frontend/SourceFile.hpp"
#include "bort/Lex/Utils.hpp"
#include <cul/cul.hpp>

namespace bort {

void Preprocessor::preprocess(const std::shared_ptr<SourceFile>& file) {
  SourceFileIt pos{ file };

  while (pos) {

    // Line comments
    if (startsWith(pos, "//")) {
      auto commentStart{ pos };
      pos += 2;
      while (*pos != '\n') {
        ++pos;
      }
      file->getBuffer().erase(commentStart.asBufIter(), pos.asBufIter());
      pos = commentStart; // avoid iterating again it should point to
                          // new '\n' pos
      continue;
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
      file->getBuffer().erase(blockCommentStart.asBufIter(),
                              pos.asBufIter());
      pos = blockCommentStart;
      continue;
    }

    // Newlines and spaces
    if (*pos == '\n' || *pos == ' ') {
      ++pos;
      continue;
    }

    if (*pos == '#') {
      auto macroLineStart{ pos };
      ++pos;
      auto macroKeyword{ consumeIdent(pos) };
      if (macroKeyword == "define") {
        defineIdentifier(pos);
      } else {
        emitError(macroLineStart, macroKeyword.length(),
                  "Unknown preprocessor directive: {}", macroKeyword);
        continue;
      }
      file->getBuffer().erase(macroLineStart.asBufIter(),
                              pos.asBufIter());
      pos = macroLineStart;
      continue;
    }

    ++pos;
  }
}

static auto readMacroDefitition(SourceFileIt& pos) -> std::string {
  std::string definition{};
  bool continueReading{ true };
  auto start{ pos };
  while (continueReading) {
    continueReading = false;
    while (*pos != '\n') {
      if (*pos == '\\') {
        ++pos;
        skipSpacesSince(pos);
        continueReading = (*pos == '\n');
        continue;
      }
      definition += *pos;
      ++pos;
    }
    ++pos;
  }
  return definition;
}

void Preprocessor::defineIdentifier(SourceFileIt& pos) {
  skipSpacesSince(pos);
  auto macroNamePos{ pos };
  auto macroName{ std::string{ consumeIdent(pos) } };
  skipSpacesSince(pos);
  auto macroDefinition{ readMacroDefitition(pos) };
  if (m_MacroDefinitions.contains(macroName)) {
    emitWarning(macroNamePos, macroName.length(),
                "Macro redefinition, previous body: {}",
                m_MacroDefinitions[macroName]);
  }
  m_MacroDefinitions[macroName] = macroDefinition;
  DEBUG_OUT("define {} = {}", macroName, macroDefinition);
}

} // namespace bort
