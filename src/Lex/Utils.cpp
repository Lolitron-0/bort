#include "bort/Lex/Utils.hpp"
#include <cctype>

namespace bort {

auto startsWith(const SourceFileIt& pos,
                std::string_view prefix) -> bool {
  if (!pos.canAdd(prefix.size())) {
    return false;
  }

  return std::equal(prefix.begin(), prefix.end(), pos.asBufIter());
}

void skipSpacesSince(SourceFileIt& pos) {
  while (*pos == ' ') {
    ++pos;
  }
}

auto consumeIdent(SourceFileIt& pos) -> std::string_view {
  size_t length{ 0 };
  auto start{ pos };

  if (!std::isalpha(*pos) && *pos != '_') {
    return {};
  }

  // TODO: actually there are more sophisticated rules in acceptable
  // identifier characters, but we'll leave it for now
  while (std::isalnum(*pos) || *pos == '_') {
    ++pos;
    ++length;
  }
  return start.getValue(length);
}

} // namespace bort
