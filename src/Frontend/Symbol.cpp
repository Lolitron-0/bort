#include "bort/Frontend/Symbol.hpp"
#include <fmt/color.h>
#include <fmt/core.h>
#include <sstream>

namespace bort {

static constexpr fmt::text_style s_ObjectStyle{ fmt::fg(
    fmt::color::dodger_blue) };

void Symbol::dump() const {
  std::cerr << toString();
}

auto Variable::toString() const -> std::string {
  std::ostringstream ss;
  ss << fmt::format(s_ObjectStyle, "Variable: ");
  ss << m_Type->toString();
  ss << fmt::format(s_ObjectStyle, " {}", m_Name);
  return ss.str();
}

auto Function::toString() const -> std::string {
  std::ostringstream ss;
  ss << fmt::format(s_ObjectStyle, "Function: ");
  ss << m_ReturnType->toString();
  ss << fmt::format(s_ObjectStyle, " {}(", m_Name);
  for (auto&& argTy : m_ArgTypes) {
    ss << argTy->toString();
    ss << fmt::format(s_ObjectStyle, ",");
  }
  ss << fmt::format(s_ObjectStyle, ")");
  return ss.str();
}

} // namespace bort
