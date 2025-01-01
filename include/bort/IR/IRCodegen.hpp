#pragma once
#include "bort/AST/ASTNode.hpp"
#include "bort/AST/BinOpExpr.hpp"
#include "bort/AST/IfStmtNode.hpp"
#include "bort/AST/NumberExpr.hpp"
#include "bort/AST/VariableExpr.hpp"
#include "bort/AST/Visitors/ASTVisitor.hpp"
#include "bort/IR/BasicBlock.hpp"
#include "bort/IR/Instruction.hpp"
#include "bort/IR/Module.hpp"

namespace bort::ir {

class IRCodegen {
public:
  [[nodiscard]] auto takeInstructions() -> Module {
    return std::move(m_Module);
  }

  void codegen(const Ref<ast::ASTRoot>& ast);

private:
  auto genericVisit(const Ref<ast::Node>& node) -> ValueRef;

  auto visit(const Ref<ast::ASTRoot>& rootNode) -> ValueRef;
  auto visit(const Ref<ast::BinOpExpr>& binopNode) -> ValueRef;
  auto visit(const Ref<ast::NumberExpr>& numNode) -> ValueRef;
  auto visit(const Ref<ast::VariableExpr>& varNode) -> ValueRef;
  auto visit(const Ref<ast::VarDecl>& varDeclNode) -> ValueRef;
  auto visit(const Ref<ast::FunctionDecl>& functionDeclNode) -> ValueRef;
  auto visit(const Ref<ast::ExpressionStmt>& expressionStmtNode)
      -> ValueRef;
  auto visit(const Ref<ast::Block>& blockNode) -> ValueRef;
  auto visit(const Ref<ast::IfStmtNode>& ifStmtNode) -> ValueRef;

  auto addInstruction(Ref<Instruction> instruction) -> ValueRef;
  void pushBB(std::string name = "");

private:
  Module m_Module;
};

} // namespace bort::ir
