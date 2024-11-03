#include "bort/AST/ASTNode.hpp"
#include "bort/AST/ASTVisitor.hpp"
#include "bort/Basic/Assert.hpp"
#include "bort/Lex/Token.hpp"
#include "cul/BiMap.hpp"
#include <boost/range.hpp>
#include <boost/range/adaptor/indexed.hpp>
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
      .Case(NodeKind::BinOpExpr, "BinOpExpr")
      .Case(NodeKind::Block, "Block")
      .Case(NodeKind::ASTRoot, "ASTRoot");
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

static auto red(std::string_view str) -> std::string {
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
void dump(int depth, std::string_view name, Node* value) {
  printDepthPrefix(depth + 1);
  fmt::print(stderr, fmt::fg(fmt::color::cyan), "{}:\n", name);
  value->dump(depth + 2);
}

} // namespace internal

void Node::dump(int depth) const {
  printDepthPrefix(depth);
  auto nodeName{ s_NodeKindNames.FindByFirst(m_Kind) };
  bort_assert(nodeName.has_value(),
              "Forgot to add node type to name mapping");
  fmt::print(stderr, fmt::fg(fmt::color::lime), "{}:\n",
             nodeName.value());
};

void ExpressionNode::dump(int depth) const {
  Node::dump(depth);
  internal::dump(depth, "Type",
                 isTypeResolved() ? m_Type->toString()
                                  : red("unresolved"));
}

void NumberExpr::dump(int depth) const {
  ExpressionNode::dump(depth);
  internal::dump(depth, "Value", m_Value);
}

void VariableExpr::dump(int depth) const {
  ExpressionNode::dump(depth);
  internal::dump(depth, "Variable ",
                 isResolved() ? m_Variable->getName()
                              : red("unresolved"));
}

void BinOpExpr::dump(int depth) const {
  ExpressionNode::dump(depth);
  internal::dump(depth, "Op",
                 Token::TokenNameMapping.Find(m_Op).value_or("UNKNOWN"));
  internal::dump(depth, "LHS", m_Lhs.get());
  internal::dump(depth, "RHS", m_Rhs.get());
}

void Block::dump(int depth) const {
  Node::dump(depth);
  for (auto&& child : m_Body) {
    internal::dump(depth, "", child.get());
  }
}

void ASTRoot::dump(int depth) const {
  Node::dump(depth);
  for (auto&& el : m_Children | boost::adaptors::indexed()) {
    internal::dump(depth - 1, fmt::format("#{}", el.index()),
                   el.value().get());
  }
}

void NumberExpr::preOrderVisit(
    const std::shared_ptr<ASTVisitor>& visitor) {
  visitor->visit(this);
}
void NumberExpr::postOrderVisit(
    const std::shared_ptr<ASTVisitor>& visitor) {
  visitor->visit(this);
}
void VariableExpr::preOrderVisit(
    const std::shared_ptr<ASTVisitor>& visitor) {
  visitor->visit(this);
}
void VariableExpr::postOrderVisit(
    const std::shared_ptr<ASTVisitor>& visitor) {
  visitor->visit(this);
}
void BinOpExpr::preOrderVisit(
    const std::shared_ptr<ASTVisitor>& visitor) {
  visitor->visit(this);
  m_Lhs->preOrderVisit(visitor);
  m_Rhs->preOrderVisit(visitor);
}
void BinOpExpr::postOrderVisit(
    const std::shared_ptr<ASTVisitor>& visitor) {
  m_Lhs->postOrderVisit(visitor);
  m_Rhs->postOrderVisit(visitor);
  visitor->visit(this);
}
void Node::structureAwareVisit(const Ref<ASTVisitor>& visitor) {
  // i guess it's better than copy pasting same implementation override
  switch (m_Kind) {
  case NodeKind::NumberExpr:
    bort_assert_nomsg(dynamic_cast<NumberExpr*>(this));
    visitor->visit(dynamic_cast<NumberExpr*>(this));
    break;
  case NodeKind::VariableExpr:
    bort_assert_nomsg(dynamic_cast<VariableExpr*>(this));
    visitor->visit(dynamic_cast<VariableExpr*>(this));
    break;
  case NodeKind::BinOpExpr:
    bort_assert_nomsg(dynamic_cast<BinOpExpr*>(this));
    visitor->visit(dynamic_cast<BinOpExpr*>(this));
    break;
  case NodeKind::Block:
    bort_assert_nomsg(dynamic_cast<Block*>(this));
    visitor->visit(dynamic_cast<Block*>(this));
    break;
  case NodeKind::ASTRoot:
    bort_assert_nomsg(dynamic_cast<ASTRoot*>(this));
    visitor->visit(dynamic_cast<ASTRoot*>(this));
    break;
  default:
    bort_assert(false, "Not implemented");
    break;
  }
}

void Block::preOrderVisit(const Ref<ASTVisitor>& visitor) {
  visitor->visit(this);
  for (auto&& child : m_Body) {
    child->preOrderVisit(visitor);
  }
}
void Block::postOrderVisit(const Ref<ASTVisitor>& visitor) {
  for (auto&& child : m_Body) {
    child->postOrderVisit(visitor);
  }
  visitor->visit(this);
}

void ASTRoot::preOrderVisit(const Ref<ASTVisitor>& visitor) {
  visitor->visit(this);
  for (auto&& child : m_Children) {
    child->preOrderVisit(visitor);
  }
}

void ASTRoot::postOrderVisit(const Ref<ASTVisitor>& visitor) {
  for (auto&& child : m_Children) {
    child->postOrderVisit(visitor);
  }
  visitor->visit(this);
}

} // namespace bort::ast
