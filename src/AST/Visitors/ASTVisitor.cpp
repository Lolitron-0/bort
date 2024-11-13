#include "bort/AST/Visitors/ASTVisitor.hpp"
#include "bort/AST/ASTNode.hpp"
#include "bort/AST/BinOpExpr.hpp"
#include "bort/AST/Block.hpp"
#include "bort/AST/ExpressionStmt.hpp"
#include "bort/AST/FunctionDecl.hpp"
#include "bort/AST/NumberExpr.hpp"
#include "bort/AST/VarDecl.hpp"
#include "bort/AST/VariableExpr.hpp"

namespace bort::ast {

void StructureAwareASTVisitor::visit(const Ref<ASTRoot>& rootNode) {
  for (auto&& child : rootNode->getChildren()) {
    genericVisit(child);
  }
}

void StructureAwareASTVisitor::visit(const Ref<BinOpExpr>& binopNode) {
  genericVisit(binopNode->getLhs());
  genericVisit(binopNode->getRhs());
}

void StructureAwareASTVisitor::visit(
    const Ref<FunctionDecl>& functionDeclNode) {
  genericVisit(functionDeclNode->getBody());
}

void StructureAwareASTVisitor::visit(
    const Ref<ExpressionStmt>& exprStmtNode) {
  genericVisit(exprStmtNode->getExpression());
}

void StructureAwareASTVisitor::visit(const Ref<Block>& blockNode) {
  for (auto&& child : blockNode->getBody()) {
    genericVisit(child);
  }
}

void StructureAwareASTVisitor::SAVisit(const Ref<ASTRoot>& node) {
  setASTRoot(node);
  genericVisit(node);
}

void StructureAwareASTVisitor::genericVisit(const Ref<Node>& node) {
  switch (node->getKind()) {
  case NodeKind::NumberExpr:
    bort_assert_nomsg(std::dynamic_pointer_cast<NumberExpr>(node));
    visit(std::dynamic_pointer_cast<NumberExpr>(node));
    break;
  case NodeKind::VariableExpr:
    bort_assert_nomsg(std::dynamic_pointer_cast<VariableExpr>(node));
    visit(std::dynamic_pointer_cast<VariableExpr>(node));
    break;
  case NodeKind::BinOpExpr:
    bort_assert_nomsg(std::dynamic_pointer_cast<BinOpExpr>(node));
    visit(std::dynamic_pointer_cast<BinOpExpr>(node));
    break;
  case NodeKind::VarDecl:
    bort_assert_nomsg(std::dynamic_pointer_cast<VarDecl>(node));
    visit(std::dynamic_pointer_cast<VarDecl>(node));
    break;
  case NodeKind::FunctionDecl:
    bort_assert_nomsg(std::dynamic_pointer_cast<FunctionDecl>(node));
    visit(std::dynamic_pointer_cast<FunctionDecl>(node));
    break;
  case NodeKind::ExpressionStmt:
    bort_assert_nomsg(std::dynamic_pointer_cast<ExpressionStmt>(node));
    visit(std::dynamic_pointer_cast<ExpressionStmt>(node));
    break;
  case NodeKind::Block:
    bort_assert_nomsg(std::dynamic_pointer_cast<Block>(node));
    visit(std::dynamic_pointer_cast<Block>(node));
    break;
  case NodeKind::ASTRoot:
    bort_assert_nomsg(std::dynamic_pointer_cast<ASTRoot>(node));
    visit(std::dynamic_pointer_cast<ASTRoot>(node));
    break;
  default:
    bort_assert(false, "Not implemented");
    break;
  }
}

auto ASTVisitorBase::getNodeDebugInfo(const Ref<Node>& node) const
    -> ASTDebugInfo {
  return m_ASTRoot->getNodeDebugInfo(node);
}

} // namespace bort::ast
