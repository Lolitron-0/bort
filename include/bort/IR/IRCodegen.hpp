#pragma once
#include "bort/AST/ASTNode.hpp"
#include "bort/AST/BinOpExpr.hpp"
#include "bort/AST/BreakStmt.hpp"
#include "bort/AST/ContinueStmt.hpp"
#include "bort/AST/FunctionCallExpr.hpp"
#include "bort/AST/IfStmt.hpp"
#include "bort/AST/IndexationExpr.hpp"
#include "bort/AST/InitializerList.hpp"
#include "bort/AST/NumberExpr.hpp"
#include "bort/AST/ReturnStmt.hpp"
#include "bort/AST/UnaryOpExpr.hpp"
#include "bort/AST/VariableExpr.hpp"
#include "bort/AST/Visitors/ASTVisitor.hpp"
#include "bort/AST/WhileStmt.hpp"
#include "bort/Frontend/Type.hpp"
#include "bort/IR/BranchInst.hpp"
#include "bort/IR/Instruction.hpp"
#include "bort/IR/Module.hpp"
#include <memory>
#include <type_traits>

namespace bort::ir {

class IRCodegen {
public:
  [[nodiscard]] auto takeInstructions() -> Module&& {
    return std::move(m_Module);
  }

  void codegen(const Ref<ast::ASTRoot>& ast);

private:
  auto genericVisit(const Ref<ast::Node>& node) -> ValueRef;

  auto visit(const Ref<ast::ASTRoot>& rootNode) -> ValueRef;
  auto visit(const Ref<ast::BinOpExpr>& binopNode) -> ValueRef;
  auto visit(const Ref<ast::UnaryOpExpr>& unaryOpExpr) -> ValueRef;
  auto visit(const Ref<ast::NumberExpr>& numNode) -> ValueRef;
  auto visit(const Ref<ast::VariableExpr>& varNode) -> ValueRef;
  auto visit(const Ref<ast::VarDecl>& varDeclNode) -> ValueRef;
  auto visit(const Ref<ast::InitializerList>& initializerListNode)
      -> ValueRef;
  auto visit(const Ref<ast::IndexationExpr>& indexationNode) -> ValueRef;
  auto visit(const Ref<ast::FunctionDecl>& functionDeclNode) -> ValueRef;
  auto visit(const Ref<ast::ExpressionStmt>& expressionStmtNode)
      -> ValueRef;
  auto visit(const Ref<ast::Block>& blockNode) -> ValueRef;
  auto visit(const Ref<ast::IfStmt>& ifStmtNode) -> ValueRef;
  auto visit(const Ref<ast::WhileStmt>& whileStmtNode) -> ValueRef;
  auto visit(const Ref<ast::ReturnStmt>& returnStmt) -> ValueRef;
  auto visit(const Ref<ast::BreakStmt>& breakStmt) -> ValueRef;
  auto visit(const Ref<ast::ContinueStmt>& continueStmt) -> ValueRef;
  auto visit(const Ref<ast::FunctionCallExpr>& funcCallExpr) -> ValueRef;

  auto genBranchFromCondition(const Ref<ast::ExpressionNode>& cond,
                              bool negate = false) -> Ref<BranchInst>;
  auto genArrayPtr(const ValueRef& arr)
      -> std::pair<Ref<PointerType>, ValueRef>;

  template <typename T>
    requires std::is_base_of_v<Instruction, T>
  auto addInstruction(Ref<T> instruction) -> Ref<T> {
    auto result{ std::dynamic_pointer_cast<T>(
        m_Module.addInstruction(std::move(instruction))) };
    processNewInst(result);
    return result;
  }

  void processNewInst(const Ref<Instruction>& instruction);
  void pushBB(std::string postfix = "", std::string name = "");

private:
  Module m_Module;
  Ref<ast::ASTRoot> m_ASTRoot;
};

} // namespace bort::ir
