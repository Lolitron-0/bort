#include "bort/AST/ASTVisitor.hpp"
#include "bort/AST/ASTNode.hpp"

namespace bort::ast {

void StructureAwareASTVisitor::visit(ASTRoot* rootNode) {
  for (auto&& child : rootNode->getChildren()) {
    child->structureAwareVisit(shared_from_this());
  }
}

void StructureAwareASTVisitor::visit(BinOpExpr* binopNode) {
  binopNode->getLhs()->structureAwareVisit(shared_from_this());
  binopNode->getRhs()->structureAwareVisit(shared_from_this());
}

void StructureAwareASTVisitor::visit(Block* blockNode) {
  for (auto&& child : blockNode->getBody()) {
    child->structureAwareVisit(shared_from_this());
  }
}

} // namespace bort::ast
