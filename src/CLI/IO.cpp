#include "bort/CLI/IO.hpp"
#include <iostream>
#define OOF_IMPL
#include <oof/oof.h>

namespace bort {

using namespace std::string_view_literals;

static constexpr oof::color ErrrorFg{ 255, 0, 0 };
static constexpr oof::color WarningFg{ 252, 122, 23 };
static constexpr oof::color DebugFg{ 79, 232, 181 };

void EmitError(const std::string& message) {
  std::cerr << oof::fg_color(ErrrorFg)
            << "error: " << oof::reset_formatting() << message
            << std::endl;
}

void EmitWarning(const std::string& message) {
  std::cerr << oof::fg_color(WarningFg)
            << "warning: " << oof::reset_formatting() << message
            << std::endl;
}

void DebugOut(const std::string& message) {
  std::cerr << oof::fg_color(DebugFg)
            << "debug: " << oof::reset_formatting() << message
            << std::endl;
}

} // namespace bort
