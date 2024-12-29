#include "bort/IR/IRCodegen.hpp"
#include "bort/AST/Visitors/Utils.hpp"
#include "bort/IR/AllocaInst.hpp"
#include "bort/IR/Constant.hpp"
#include "bort/IR/MoveInst.hpp"
#include "bort/IR/OpInst.hpp"
#include "bort/Lex/Token.hpp"

namespace bort::ir {

auto IRCodegen::genIncrementedName() -> std::string {
  static size_t nameCounter = 0;
  return fmt::format("{}", nameCounter++);
}

void IRCodegen::codegen(const Ref<ast::ASTRoot>& ast) {
  genericVisit(ast);
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
    m_Instructions.push_back(
        makeRef<MoveInst>(std::move(lhs), std::move(rhs)));
    return m_Instructions.back();
  }

  m_Instructions.push_back(makeRef<OpInst>(
      binOpNode->getOp(),
      Value::createUnique(binOpNode->getType(),
                          IRCodegen::genIncrementedName()),
      lhs, rhs));
  return m_Instructions.back();
}

auto IRCodegen::visit(const Ref<ast::NumberExpr>& numberNode)
    -> ValueRef {
  return IntConstant::create(numberNode->getValue());
}

auto IRCodegen::visit(const Ref<ast::VariableExpr>& varNode) -> ValueRef {
  return Value::get(varNode->getVarName());
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
  m_Instructions.push_back(
      makeRef<AllocaInst>(varSymbol->getType(), varSymbol->getName(),
                          elementSize, IntConstant::create(1)));
  return m_Instructions.back();
}

auto IRCodegen::visit(const Ref<ast::FunctionDecl>& functionDeclNode)
    -> ValueRef {

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

auto IRCodegen::visit(const Ref<ast::IfStmtNode>& ifStmtNode)
    -> ValueRef {
  genericVisit(ifStmtNode->getCondition());
  genericVisit(ifStmtNode->getThenBlock());
  genericVisit(ifStmtNode->getElseBlock());
  return nullptr;
}
} // namespace bort::ir
