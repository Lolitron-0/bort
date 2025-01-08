#include "bort/AST/Visitors/ASTVisitor.hpp"
#include "bort/AST/ASTNode.hpp"
#include "bort/AST/BinOpExpr.hpp"
#include "bort/AST/Block.hpp"
#include "bort/AST/ExpressionStmt.hpp"
#include "bort/AST/FunctionDecl.hpp"
#include "bort/AST/Visitors/Utils.hpp"

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

void StructureAwareASTVisitor::visit(const Ref<IfStmt>& ifStmtNode) {
  genericVisit(ifStmtNode->getCondition());
  genericVisit(ifStmtNode->getThenBlock());
  genericVisit(ifStmtNode->getElseBlock());
}

void StructureAwareASTVisitor::SAVisit(const Ref<ASTRoot>& node) {
  setASTRoot(node);
  genericVisit(node);
}

void StructureAwareASTVisitor::genericVisit(const Ref<Node>& node) {
  callHandler(node, [this](const auto& node) {
    visit(node);
  });
}

auto ASTVisitorBase::getNodeDebugInfo(const Ref<Node>& node) const
    -> ASTDebugInfo {
  return m_ASTRoot->getNodeDebugInfo(node);
}

void StructureAwareASTVisitor::visit(
    const Ref<WhileStmt>& whileStmtNode) {
  genericVisit(whileStmtNode->getCondition());
  genericVisit(whileStmtNode->getBody());
};
} // namespace bort::ast
