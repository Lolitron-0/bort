#include "bort/AST/ASTNode.hpp"
#include "bort/AST/BinOpExpr.hpp"
#include "bort/AST/Block.hpp"
#include "bort/AST/DumpCommons.hpp"
#include "bort/AST/NumberExpr.hpp"
#include "bort/AST/VarDecl.hpp"
#include "bort/AST/VariableExpr.hpp"
#include "bort/Basic/Assert.hpp"
#include "cul/BiMap.hpp"
#include <boost/range.hpp>
#include <boost/range/adaptor/indexed.hpp>

namespace bort::ast {

static constexpr cul::BiMap s_NodeKindNames{ [](auto&& selector) {
  return selector.Case(NodeKind::NumberExpr, "NumberExpr")
      .Case(NodeKind::VariableExpr, "VariableExpr")
      .Case(NodeKind::StringExpr, "StringExpr")
      .Case(NodeKind::CharExpr, "CharExpr")
      .Case(NodeKind::BinOpExpr, "BinOpExpr")
      .Case(NodeKind::VarDecl, "VarDecl")
      .Case(NodeKind::Block, "Block")
      .Case(NodeKind::ASTRoot, "ASTRoot");
} };

void Node::dump(int depth) const {
  printDepthPrefix(depth);
  auto nodeName{ s_NodeKindNames.FindByFirst(m_Kind) };
  bort_assert(nodeName.has_value(),
              "Forgot to add node type to name mapping");
  fmt::print(stderr, fmt::fg(fmt::color::lime), "{}:\n",
             nodeName.value());
};

void Node::structureAwareVisit(
    const Ref<StructureAwareASTVisitor>& visitor) {
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
  case NodeKind::VarDecl:
    bort_assert_nomsg(dynamic_cast<VarDecl*>(this));
    visitor->visit(dynamic_cast<VarDecl*>(this));
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
