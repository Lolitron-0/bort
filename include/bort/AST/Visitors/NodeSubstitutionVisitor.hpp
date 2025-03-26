#pragma once
#include "bort/AST/ASTNode.hpp"
#include "bort/AST/BinOpExpr.hpp"
#include "bort/AST/FunctionCallExpr.hpp"
#include "bort/AST/NumberExpr.hpp"
#include "bort/AST/UnaryOpExpr.hpp"
#include "bort/AST/Visitors/ASTVisitor.hpp"
#include <concepts>

namespace bort::ast {

class NodeSubstitutionVisitor : public StructureAwareASTVisitor {
private:
  void visit(const Ref<UnaryOpExpr>& unaryOpNode) override;
};

} // namespace bort::ast
