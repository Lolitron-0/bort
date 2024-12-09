#pragma once

#include "bort/AST/ASTNode.hpp"
#include "bort/AST/BinOpExpr.hpp"
#include "bort/AST/ExpressionStmt.hpp"
#include "bort/AST/FunctionDecl.hpp"
#include "bort/AST/NumberExpr.hpp"
#include "bort/AST/VarDecl.hpp"
#include "bort/AST/VariableExpr.hpp"
namespace bort::ast {

/// \brief Driver for all AST walkers
///
/// It is used in StructureAwareASTVisitor which is aimed on more or less
/// stateless traversals, when some heavy state forwarding is needed,
/// walkers implement StructureAwareASTVisitor::genericVisit themselves to
/// handle return values and other signarure changes. However, these
/// visitors have to overload handler for every node type
template <typename F>
auto callHandler(const Ref<Node>& node, F&& visit) {
  switch (node->getKind()) {
  case NodeKind::NumberExpr:
    bort_assert_nomsg(std::dynamic_pointer_cast<NumberExpr>(node));
    return visit(std::dynamic_pointer_cast<NumberExpr>(node));
    break;
  case NodeKind::VariableExpr:
    bort_assert_nomsg(std::dynamic_pointer_cast<VariableExpr>(node));
    return visit(std::dynamic_pointer_cast<VariableExpr>(node));
    break;
  case NodeKind::BinOpExpr:
    bort_assert_nomsg(std::dynamic_pointer_cast<BinOpExpr>(node));
    return visit(std::dynamic_pointer_cast<BinOpExpr>(node));
    break;
  case NodeKind::VarDecl:
    bort_assert_nomsg(std::dynamic_pointer_cast<VarDecl>(node));
    return visit(std::dynamic_pointer_cast<VarDecl>(node));
    break;
  case NodeKind::FunctionDecl:
    bort_assert_nomsg(std::dynamic_pointer_cast<FunctionDecl>(node));
    return visit(std::dynamic_pointer_cast<FunctionDecl>(node));
    break;
  case NodeKind::ExpressionStmt:
    bort_assert_nomsg(std::dynamic_pointer_cast<ExpressionStmt>(node));
    return visit(std::dynamic_pointer_cast<ExpressionStmt>(node));
    break;
  case NodeKind::Block:
    bort_assert_nomsg(std::dynamic_pointer_cast<Block>(node));
    return visit(std::dynamic_pointer_cast<Block>(node));
    break;
  case NodeKind::ASTRoot:
    bort_assert_nomsg(std::dynamic_pointer_cast<ASTRoot>(node));
    return visit(std::dynamic_pointer_cast<ASTRoot>(node));
    break;
  default:
    bort_assert(false, "Not implemented");
    // unreachable, casting to root just for fun
    return visit(std::dynamic_pointer_cast<ASTRoot>(node));
    break;
  }
}

} // namespace bort::ast
