#pragma once
#include "bort/AST/ASTNode.hpp"
#include <fmt/base.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/ranges.h>

namespace bort::ast {

inline auto depthPrefix(int depth) -> std::string {

  std::string res{};
  for (int i{ 0 }; i < depth; i++) {
    res += "|  ";
  }
  return res;
}

inline void printDepthPrefix(int depth) {
  fmt::print(stderr, fmt::fg(fmt::color::dark_gray), "{}",
             depthPrefix(depth));
}

inline auto red(std::string_view str) -> std::string {
  return fmt::format(fmt::fg(fmt::color::fire_brick), "{}", str);
}

namespace internal {

template <typename T>
void dump(int depth, std::string_view name, T value) {
  printDepthPrefix(depth + 1);
  fmt::print(stderr, fmt::fg(fmt::color::orange), "{} = {}\n", name,
             value);
}

template <>
inline void dump(int depth, std::string_view name, Node* value) {
  printDepthPrefix(depth + 1);
  fmt::print(stderr, fmt::fg(fmt::color::cyan), "{}:\n", name);
  value->dump(depth + 2);
}

} // namespace internal

} // namespace bort::ast
