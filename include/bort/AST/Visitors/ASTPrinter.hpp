#pragma once
#include "bort/AST/Visitors/ASTVisitor.hpp"
#include <fmt/base.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/ranges.h>

namespace bort::ast {

class ExpressionNode;

class ASTPrinter : public StructureAwareASTVisitor {
private:
  void visit(const Ref<ASTRoot>& rootNode) override;

  void visit(const Ref<NumberExpr>& numNode) override;
  void visit(const Ref<VariableExpr>& varNode) override;
  void visit(const Ref<StringExpr>& strNode) override;
  void visit(const Ref<CharExpr>& charNode) override;
  void visit(const Ref<VarDecl>& varDeclNode) override;
  void visit(const Ref<BinOpExpr>& binopNode) override;
  void visit(const Ref<Block>& blockNode) override;

  void push();
  void pop();

  void printDepthPrefix() const;

  template <typename T>
  void dump(std::string_view name, T value) {
    push();
    printDepthPrefix();
    fmt::print(stderr, fmt::fg(fmt::color::orange), "{} = ", name);
    fmt::print(stderr, "{}\n", value);
    pop();
  }

  void dumpNodeInfo(const Ref<Node>& node);
  void dumpExprInfo(const Ref<ExpressionNode>& node);
  void dump(std::string_view name, const Ref<Node>& child);

  int m_Depth{ 0 };
};

} // namespace bort::ast
