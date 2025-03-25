#pragma once

#include "bort/AST/ASTNode.hpp"
#include "bort/AST/BinOpExpr.hpp"
#include "bort/AST/ExpressionStmt.hpp"
#include "bort/AST/FunctionCallExpr.hpp"
#include "bort/AST/FunctionDecl.hpp"
#include "bort/AST/IfStmt.hpp"
#include "bort/AST/NumberExpr.hpp"
#include "bort/AST/ReturnStmt.hpp"
#include "bort/AST/UnaryOpExpr.hpp"
#include "bort/AST/VarDecl.hpp"
#include "bort/AST/VariableExpr.hpp"
#include "bort/AST/WhileStmt.hpp"
#include "bort/Basic/Casts.hpp"
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
    bort_assert_nomsg(dynCastRef<NumberExpr>(node));
    return visit(dynCastRef<NumberExpr>(node));
    break;
  case NodeKind::VariableExpr:
    bort_assert_nomsg(dynCastRef<VariableExpr>(node));
    return visit(dynCastRef<VariableExpr>(node));
    break;
  case NodeKind::BinOpExpr:
    bort_assert_nomsg(dynCastRef<BinOpExpr>(node));
    return visit(dynCastRef<BinOpExpr>(node));
    break;
  case NodeKind::UnaryOpExpr:
    bort_assert_nomsg(dynCastRef<UnaryOpExpr>(node));
    return visit(dynCastRef<UnaryOpExpr>(node));
    break;
  case NodeKind::VarDecl:
    bort_assert_nomsg(dynCastRef<VarDecl>(node));
    return visit(dynCastRef<VarDecl>(node));
    break;
  case NodeKind::FunctionDecl:
    bort_assert_nomsg(dynCastRef<FunctionDecl>(node));
    return visit(dynCastRef<FunctionDecl>(node));
    break;
  case NodeKind::FunctionCallExpr:
    bort_assert_nomsg(dynCastRef<FunctionCallExpr>(node));
    return visit(dynCastRef<FunctionCallExpr>(node));
    break;
  case NodeKind::ExpressionStmt:
    bort_assert_nomsg(dynCastRef<ExpressionStmt>(node));
    return visit(dynCastRef<ExpressionStmt>(node));
    break;
  case NodeKind::Block:
    bort_assert_nomsg(dynCastRef<Block>(node));
    return visit(dynCastRef<Block>(node));
    break;
  case NodeKind::IfStmt:
    bort_assert_nomsg(dynCastRef<IfStmt>(node));
    return visit(dynCastRef<IfStmt>(node));
    break;
  case NodeKind::WhileStmt:
    bort_assert_nomsg(dynCastRef<WhileStmt>(node));
    return visit(dynCastRef<WhileStmt>(node));
    break;
  case NodeKind::ReturnStmt:
    bort_assert_nomsg(dynCastRef<ReturnStmt>(node));
    return visit(dynCastRef<ReturnStmt>(node));
    break;
  case NodeKind::ASTRoot:
    bort_assert_nomsg(dynCastRef<ASTRoot>(node));
    return visit(dynCastRef<ASTRoot>(node));
    break;
  default:
    bort_assert(false, "Generic visit not implemented for node");
    // unreachable, casting to root just for fun
    return visit(dynCastRef<ASTRoot>(node));
    break;
  }
}

} // namespace bort::ast
