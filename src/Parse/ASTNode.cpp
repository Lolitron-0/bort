#include "bort/Parse/ASTNode.hpp"
#include "bort/Lex/Token.hpp"
#include "cul/BiMap.hpp"
#include <fmt/base.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <memory>
#include <string_view>

namespace bort::ast {

static constexpr cul::BiMap s_NodeKindNames{ [](auto&& selector) {
  return selector.Case(NodeKind::NumberExpr, "NumberExpr")
      .Case(NodeKind::VariableExpr, "VariableExpr")
      .Case(NodeKind::StringExpr, "StringExpr")
      .Case(NodeKind::CharExpr, "CharExpr")
      .Case(NodeKind::BinOpExpr, "BinOpExpr");
} };

static auto depthPrefix(int depth) -> std::string {

  std::string res{};
  for (int i{ 0 }; i < depth; i++) {
    res += "|  ";
  }
  return res;
}

static void printDepthPrefix(int depth) {
  fmt::print(stderr, fmt::fg(fmt::color::dark_gray), "{}",
             depthPrefix(depth));
}

namespace internal {

template <typename T>
void dump(int depth, std::string_view name, T value) {
  printDepthPrefix(depth + 1);
  fmt::print(stderr, fmt::fg(fmt::color::orange), "{} = {}\n", name,
             value);
}

template <>
void dump(int depth, std::string_view name, Node* value) {
  printDepthPrefix(depth + 1);
  fmt::print(stderr, fmt::fg(fmt::color::cyan), "{}:\n", name);
  value->dump(depth + 2);
}

} // namespace internal

void Node::dump(int depth) const {
  printDepthPrefix(depth);
  auto nodeName{ s_NodeKindNames.FindByFirst(m_Kind) };
  assert(nodeName.has_value() &&
         "Forgot to add node type to name mapping");
  fmt::print(stderr, fmt::fg(fmt::color::lime), "{}:\n",
             nodeName.value());
};

void NumberExpr::dump(int depth) const {
  Node::dump(depth);
  internal::dump(depth, "Value", m_Value);
}

void VariableExpr::dump(int depth) const {
  Node::dump();
  internal::dump(depth, "Name", m_Name);
}

void BinOpExpr::dump(int depth) const {
  Node::dump(depth);
  internal::dump(
      depth, "Op",
      Token::TokenNameMapping.Find(m_Op).value_or("UNKNOWN"));
  internal::dump(depth, "LHS", m_Lhs.get());
  internal::dump(depth, "RHS", m_Rhs.get());
}

} // namespace bort::ast
