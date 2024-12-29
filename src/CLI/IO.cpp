#include "bort/CLI/IO.hpp"
#include "bort/Frontend/SourceFile.hpp"

namespace bort {

Diagnostic::Level Diagnostic::s_Level{ Level::All };

void Diagnostic::underlineSource(FILE* out, const SourceFileIt& loc,
                                 size_t length, fmt::color color) {
  fmt::print(out, "at {}:\n", loc.toString());

  fmt::println(out, "{}", loc.getCurrentLine());

  fmt::print(out, fmt::fg(color), "{}^{}\n",
             std::string(loc.getColumnNum() - 2, ' '),
             std::string(length - 1, '~'));
}

} // namespace bort
