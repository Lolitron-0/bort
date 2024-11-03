#include "bort/Frontend/Type.hpp"
#include <fmt/color.h>
#include <fmt/core.h>
#include <sstream>

namespace bort {

static constexpr fmt::text_style s_TypeStyle{ fmt::fg(
    fmt::color::fuchsia) };

void Type::dump() const {
  fmt::print(stderr, "{}", toString());
};

auto VoidType::toString() const -> std::string {
  std::ostringstream ss;
  ss << fmt::format(s_TypeStyle, "void");
  return ss.str();
}

auto IntType::toString() const -> std::string {
  std::ostringstream ss;
  ss << fmt::format(s_TypeStyle, "int");
  return ss.str();
}

auto CharType::toString() const -> std::string {
  std::ostringstream ss;
  fmt::format(s_TypeStyle, "char");
  return ss.str();
}

auto PointerType::toString() const -> std::string {
  std::ostringstream ss;
  ss << m_Pointee->toString();
  ss << fmt::format(s_TypeStyle, "*");
  return ss.str();
}

auto ArrayType::toString() const -> std::string {
  std::ostringstream ss;
  ss << m_BaseType->toString();
  ss << fmt::format(s_TypeStyle, "[{}]", m_NumElements);
  return ss.str();
}

} // namespace bort
