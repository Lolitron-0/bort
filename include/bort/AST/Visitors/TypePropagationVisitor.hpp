#pragma once
#include "bort/AST/BinOpExpr.hpp"
#include "bort/AST/Visitors/ASTVisitor.hpp"

namespace bort::ast {

class TypePropagationVisitor : public StructureAwareASTVisitor {
private:
  void visit(const Ref<BinOpExpr>& binopNode) override {
    StructureAwareASTVisitor::visit(binopNode);

    auto lhsType {binopNode->getLhs()->getType()};
    auto rhsType {binopNode->getRhs()->getType()};

    if(!lhsType || !rhsType) {
      // we failed somewhere else, so just skip this node
      return;
    }

    if(lhsType->getSizeof() > rhsType->getSizeof()) {
      binopNode->setType(lhsType);
    }
    else {
      binopNode->setType(rhsType);
    }
  }
};

} // namespace bort::ast
