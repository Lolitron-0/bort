#pragma once

#include "bort/AST/ASTNode.hpp"
#include "bort/AST/BinOpExpr.hpp"
#include "bort/AST/BreakStmt.hpp"
#include "bort/AST/ContinueStmt.hpp"
#include "bort/AST/ExpressionStmt.hpp"
#include "bort/AST/FunctionCallExpr.hpp"
#include "bort/AST/FunctionDecl.hpp"
#include "bort/AST/GotoStmt.hpp"
#include "bort/AST/IfStmt.hpp"
#include "bort/AST/IndexationExpr.hpp"
#include "bort/AST/InitializerList.hpp"
#include "bort/AST/LabelStmt.hpp"
#include "bort/AST/NumberExpr.hpp"
#include "bort/AST/ReturnStmt.hpp"
#include "bort/AST/UnaryOpExpr.hpp"
#include "bort/AST/VarDecl.hpp"
#include "bort/AST/VariableExpr.hpp"
#include "bort/AST/WhileStmt.hpp"
#include "bort/Basic/Casts.hpp"
namespace bort::ast {

template <typename NodeT, typename F>
constexpr static auto castVisit(const Ref<Node>& node, F&& visit) {
  bort_assert_nomsg(dynCastRef<NodeT>((node)));
  return visit(dynCastRef<NodeT>((node)));
}

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
    return castVisit<NumberExpr>(node, visit);
  case NodeKind::VariableExpr:
    return castVisit<VariableExpr>(node, visit);
  case NodeKind::BinOpExpr:
    return castVisit<BinOpExpr>(node, visit);
  case NodeKind::UnaryOpExpr:
    return castVisit<UnaryOpExpr>(node, visit);
  case NodeKind::VarDecl:
    return castVisit<VarDecl>(node, visit);
  case NodeKind::InitializerList:
    return castVisit<InitializerList>(node, visit);
  case NodeKind::IndexationExpr:
    return castVisit<IndexationExpr>(node, visit);
  case NodeKind::FunctionDecl:
    return castVisit<FunctionDecl>(node, visit);
  case NodeKind::FunctionCallExpr:
    return castVisit<FunctionCallExpr>(node, visit);
  case NodeKind::ExpressionStmt:
    return castVisit<ExpressionStmt>(node, visit);
  case NodeKind::Block:
    return castVisit<Block>(node, visit);
  case NodeKind::IfStmt:
    return castVisit<IfStmt>(node, visit);
  case NodeKind::WhileStmt:
    return castVisit<WhileStmt>(node, visit);
  case NodeKind::ReturnStmt:
    return castVisit<ReturnStmt>(node, visit);
  case NodeKind::BreakStmt:
    return castVisit<BreakStmt>(node, visit);
  case NodeKind::ContinueStmt:
    return castVisit<ContinueStmt>(node, visit);
  case NodeKind::LabelStmt:
    return castVisit<LabelStmt>(node, visit);
  case NodeKind::GotoStmt:
    return castVisit<GotoStmt>(node, visit);
  case NodeKind::ASTRoot:
    return castVisit<ASTRoot>(node, visit);
  default:
    bort_assert(false, "Generic visit not implemented for node");
    // unreachable, casting to root just for fun
    return visit(dynCastRef<ASTRoot>(node));
    break;
  }
}

} // namespace bort::ast
