#pragma once
#include "bort/AST/Visitors/ASTVisitor.hpp"
#include <concepts>
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
  void visit(const Ref<FunctionDecl>& functionDeclNode) override;
  void visit(const Ref<ExpressionStmt>& expressionStmtNode) override;
  void visit(const Ref<BinOpExpr>& binopNode) override;
  void visit(const Ref<Block>& blockNode) override;
  void visit(const Ref<IfStmt>& ifStmtNode) override;
  void visit(const Ref<WhileStmt>& whileStmtNode) override;
  void visit(const Ref<FunctionCallExpr>& functionCallExpr) override;

  void push();
  void pop();

  void printDepthPrefix() const;

  template <typename T>
  void dump(std::string_view name, T&& value) {
    push();
    printDepthPrefix();
    fmt::print(stderr, fmt::fg(fmt::color::orange), "{} = ", name);
    fmt::print(stderr, "{}\n", value);
    pop();
  }

  template <std::convertible_to<Ref<Node>> T>
  void dump(std::string_view name, T&& value) {
    push();
    printDepthPrefix();
    fmt::print(stderr, fmt::fg(fmt::color::cyan), "{}:\n", name);
    push();
    genericVisit(value);
    pop();
    pop();
  }

  void dumpNodeInfo(const Ref<Node>& node);
  void dumpExprInfo(const Ref<ExpressionNode>& node);

  int m_Depth{ 0 };
};

} // namespace bort::ast
