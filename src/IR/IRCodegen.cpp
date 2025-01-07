#include "bort/IR/IRCodegen.hpp"
#include "bort/AST/Visitors/Utils.hpp"
#include "bort/Basic/Assert.hpp"
#include "bort/Frontend/Type.hpp"
#include "bort/IR/AllocaInst.hpp"
#include "bort/IR/BranchInst.hpp"
#include "bort/IR/Constant.hpp"
#include "bort/IR/MoveInst.hpp"
#include "bort/IR/OpInst.hpp"
#include "bort/IR/Register.hpp"
#include "bort/IR/VariableUse.hpp"
#include "bort/Lex/Token.hpp"
#include "fmt/format.h"

namespace bort::ir {

void IRCodegen::codegen(const Ref<ast::ASTRoot>& ast) {
  genericVisit(ast);
  m_Module.revalidateBasicBlocks();
}

auto IRCodegen::genericVisit(const Ref<ast::Node>& node) -> ValueRef {
  return ast::callHandler(node, [this](const auto& node) {
    return visit(node);
  });
}

auto IRCodegen::visit(const Ref<ast::BinOpExpr>& binOpNode) -> ValueRef {
  auto lhs{ genericVisit(binOpNode->getLhs()) };
  auto rhs{ genericVisit(binOpNode->getRhs()) };

  if (binOpNode->getOp() == TokenKind::Assign) {
    auto newInst{ addInstruction(
        makeRef<MoveInst>(std::move(lhs), std::move(rhs))) };
    return newInst->getDestination();
  }

  auto newInst{ addInstruction(makeRef<OpInst>(
      binOpNode->getOp(), Register::getOrCreate(binOpNode->getType()),
      lhs, rhs)) };
  return newInst->getDestination();
}

auto IRCodegen::visit(const Ref<ast::NumberExpr>& numberNode)
    -> ValueRef {
  return IntConstant::create(numberNode->getValue());
}

auto IRCodegen::visit(const Ref<ast::VariableExpr>& varNode) -> ValueRef {
  return VariableUse::get(varNode->getVariable());
}

auto IRCodegen::visit(const Ref<ast::ASTRoot>& rootNode) -> ValueRef {
  for (auto&& child : rootNode->getChildren()) {
    genericVisit(child);
  }
  return nullptr;
}

auto IRCodegen::visit(const Ref<ast::VarDecl>& varDeclNode) -> ValueRef {
  auto varSymbol{ varDeclNode->getVariable() };
  ValueRef elementSize{ IntConstant::create(
      static_cast<int32_t>(varSymbol->getType()->getSizeof())) };
  return addInstruction(makeRef<AllocaInst>(
      std::move(varSymbol), elementSize, IntConstant::create(1)));
}

auto IRCodegen::visit(const Ref<ast::FunctionDecl>& functionDeclNode)
    -> ValueRef {
  /// @todo function types
  m_Module.addFunction(VoidType::get(),
                       functionDeclNode->getFunction()->getName());
  pushBB("", functionDeclNode->getFunction()->getName());
  return genericVisit(functionDeclNode->getBody());
}

auto IRCodegen::visit(const Ref<ast::ExpressionStmt>& expressionStmtNode)
    -> ValueRef {
  return genericVisit(expressionStmtNode->getExpression());
}

auto IRCodegen::visit(const Ref<ast::Block>& blockNode) -> ValueRef {
  for (auto&& child : blockNode->getBody()) {
    genericVisit(child);
  }
  return nullptr;
}

/// if statement is expanded to:
/// br condition L_true
/// ; else block
/// ...
/// br L_end
/// L_True:
/// ; then block
/// ...
/// L_End:
auto IRCodegen::visit(const Ref<ast::IfStmtNode>& ifStmtNode)
    -> ValueRef {
  auto condition{ genericVisit(ifStmtNode->getCondition()) };

  auto thenBr{ addInstruction(makeRef<BranchInst>(condition)) };
  bort_assert_nomsg(thenBr);

  pushBB("_false");
  genericVisit(ifStmtNode->getElseBlock());
  auto endBr{ addInstruction(makeRef<BranchInst>()) };
  bort_assert_nomsg(endBr);

  pushBB("_true");
  auto lastBBIt{ m_Module.getLastBBIt() };
  thenBr->setTarget(&*lastBBIt);
  genericVisit(ifStmtNode->getThenBlock());

  pushBB("_end");
  lastBBIt = m_Module.getLastBBIt();
  endBr->setTarget(&*lastBBIt);
  return nullptr;
}

void IRCodegen::pushBB(std::string postfix, std::string name) {
  static int s_NameCounter{ 0 };
  if (name.empty()) {
    name = fmt::format(".L{}{}", s_NameCounter++, postfix);
  } else {
    name += postfix;
  }
  m_Module.addBasicBlock(std::move(name));
}

} // namespace bort::ir
