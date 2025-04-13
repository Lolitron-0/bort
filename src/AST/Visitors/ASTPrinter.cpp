#include "bort/AST/Visitors/ASTPrinter.hpp"
#include "bort/AST/ASTNode.hpp"
#include "bort/AST/BinOpExpr.hpp"
#include "bort/AST/Block.hpp"
#include "bort/AST/ExpressionNode.hpp"
#include "bort/AST/ExpressionStmt.hpp"
#include "bort/AST/FunctionCallExpr.hpp"
#include "bort/AST/FunctionDecl.hpp"
#include "bort/AST/IfStmt.hpp"
#include "bort/AST/NumberExpr.hpp"
#include "bort/AST/ReturnStmt.hpp"
#include "bort/AST/VarDecl.hpp"
#include "bort/AST/VariableExpr.hpp"
#include "bort/AST/Visitors/ASTVisitor.hpp"
#include "bort/AST/WhileStmt.hpp"
#include "bort/Basic/Assert.hpp"
#include <boost/range/adaptors.hpp>
#include <cul/BiMap.hpp>
#include <fmt/color.h>

namespace bort::ast {

static constexpr cul::BiMap s_NodeKindNames{ [](auto&& selector) {
  return selector.Case(NodeKind::NumberExpr, "NumberExpr")
      .Case(NodeKind::VariableExpr, "VariableExpr")
      .Case(NodeKind::StringExpr, "StringExpr")
      .Case(NodeKind::InitializerList, "InitializerList")
      .Case(NodeKind::IndexationExpr, "IndexationExpr")
      .Case(NodeKind::CharExpr, "CharExpr")
      .Case(NodeKind::BinOpExpr, "BinOpExpr")
      .Case(NodeKind::UnaryOpExpr, "UnaryOpExpr")
      .Case(NodeKind::VarDecl, "VarDecl")
      .Case(NodeKind::FunctionDecl, "FunctionDecl")
      .Case(NodeKind::FunctionCallExpr, "FunctionCallExpr")
      .Case(NodeKind::ExpressionStmt, "ExpressionStmt")
      .Case(NodeKind::IfStmt, "IfStmt")
      .Case(NodeKind::WhileStmt, "WhileStmt")
      .Case(NodeKind::ReturnStmt, "ReturnStmt")
      .Case(NodeKind::BreakStmt, "BreakStmt")
      .Case(NodeKind::ContinueStmt, "ContinueStmt")
      .Case(NodeKind::LabelStmt, "LabelStmt")
      .Case(NodeKind::GotoStmt, "GotoStmt")
      .Case(NodeKind::Block, "Block")
      .Case(NodeKind::ASTRoot, "ASTRoot");
} };

inline auto depthPrefix(int depth) -> std::string {

  std::string res{};
  for (int i{ 0 }; i < depth; i++) {
    res += "| ";
  }
  return res;
}

static auto red(std::string_view str) -> std::string {
  return fmt::format(fmt::fg(fmt::color::fire_brick), "{}", str);
}

void ASTPrinter::printDepthPrefix() const {
  fmt::print(stderr,
             fmt::fg(fmt::color::dark_gray) | fmt::emphasis::faint, "{}",
             depthPrefix(m_Depth));
}

void ASTPrinter::dumpNodeInfo(const Ref<Node>& node) {
  printDepthPrefix();
  auto nodeName{ s_NodeKindNames.FindByFirst(node->getKind()) };
  bort_assert(nodeName.has_value(),
              "Forgot to add node type to name mapping");
  fmt::print(stderr, fmt::fg(fmt::color::lime), "{}:\n",
             nodeName.value());
}
void ASTPrinter::dumpExprInfo(const Ref<ExpressionNode>& node) {
  dumpNodeInfo(node);
  dump("Type", node->isTypeResolved() ? node->getType()->toString()
                                      : red("unresolved"));
}

void ASTPrinter::visit(const Ref<ASTRoot>& rootNode) {
  dumpNodeInfo(rootNode);
  for (auto&& el : rootNode->getChildren() | boost::adaptors::indexed()) {
    dump(fmt::format("#{}", el.index()), el.value());
  }
}

void ASTPrinter::visit(const Ref<NumberExpr>& numNode) {
  dumpExprInfo(numNode);
  dump("Value", numNode->getValue());
}

void ASTPrinter::visit(const Ref<VariableExpr>& varNode) {
  dumpExprInfo(varNode);
  dump("Variable ",
       varNode->getVariable()->getName() +
           (varNode->isResolved() ? "" : red(" (unresolved)")));
}

void ASTPrinter::visit(const Ref<StringExpr>& /*strNode*/) {
  bort_assert(false, "Not implemented");
}

void ASTPrinter::visit(const Ref<CharExpr>& /*charNode*/) {
  bort_assert(false, "Not implemented");
}

void ASTPrinter::visit(const Ref<VarDecl>& varDeclNode) {
  dumpNodeInfo(varDeclNode);
  dump("Variable", varDeclNode->getVariable()->toString());
  if (varDeclNode->hasInitializer()) {
    dump("Initializer", varDeclNode->getInitializer());
  }
}

void ASTPrinter::visit(const Ref<InitializerList>& initializerListNode) {
  dumpExprInfo(initializerListNode);
  for (auto&& el :
       initializerListNode->getValues() | boost::adaptors::indexed()) {
    dump(fmt::format("Value #{}", el.index()), el.value());
  }
}

void ASTPrinter::visit(const Ref<IndexationExpr>& indexationExpr) {
  dumpExprInfo(indexationExpr);
  dump("Array", indexationExpr->getArray());
  dump("Index", indexationExpr->getIndex());
}

void ASTPrinter::visit(const Ref<FunctionDecl>& functionDeclNode) {
  dumpNodeInfo(functionDeclNode);
  dump("Function", functionDeclNode->getFunction()->toString());
  dump("Body", functionDeclNode->getBody());
}

void ASTPrinter::visit(const Ref<BinOpExpr>& binopNode) {
  dumpExprInfo(binopNode);
  dump("Op", Token::TokenNameMapping.Find(binopNode->getOp())
                 .value_or("UNKNOWN"));
  dump("LHS", binopNode->getLHS());
  dump("RHS", binopNode->getRHS());
}

void ASTPrinter::visit(const Ref<UnaryOpExpr>& unaryOpNode) {
  dumpExprInfo(unaryOpNode);
  dump("Op", Token::TokenNameMapping.Find(unaryOpNode->getOp())
                 .value_or("UNKNOWN"));
  dump("Operand", unaryOpNode->getOperand());
}

void ASTPrinter::visit(const Ref<Block>& blockNode) {
  dumpNodeInfo(blockNode);
  for (auto&& el : blockNode->getBody() | boost::adaptors::indexed()) {
    dump(fmt::format("#{}", el.index()), el.value());
  }
}

void ASTPrinter::push() {
  m_Depth++;
}

void ASTPrinter::pop() {
  m_Depth--;
}

void ASTPrinter::visit(const Ref<ExpressionStmt>& expressionStmtNode) {
  dumpNodeInfo(expressionStmtNode);
  dump("Expression", expressionStmtNode->getExpression());
}

void ASTPrinter::visit(const Ref<IfStmt>& ifStmtNode) {
  dumpNodeInfo(ifStmtNode);
  dump("Condition", ifStmtNode->getCondition());
  dump("Then", ifStmtNode->getThenBlock());
  dump("Else", ifStmtNode->getElseBlock());
}

void ASTPrinter::visit(const Ref<WhileStmt>& whileStmtNode) {
  dumpNodeInfo(whileStmtNode);
  dump("Condition", whileStmtNode->getCondition());
  dump("Body", whileStmtNode->getBody());
}

void ASTPrinter::visit(const Ref<FunctionCallExpr>& functionCallExpr) {
  dumpNodeInfo(functionCallExpr);
  dump("Function",
       functionCallExpr->getFunction()->getName() +
           (functionCallExpr->isResolved() ? "" : red(" (unresolved)")));
  for (auto&& it :
       functionCallExpr->getArgs() | boost::adaptors::indexed()) {
    dump(fmt::format("Arg #{}", it.index()), it.value());
  }
}

void ASTPrinter::visit(const Ref<ReturnStmt>& returnStmtNode) {
  dumpNodeInfo(returnStmtNode);
  dump("Expression", returnStmtNode->getExpression());
}

void ASTPrinter::visit(const Ref<BreakStmt>& breakStmtNode) {
  dumpNodeInfo(breakStmtNode);
}

void ASTPrinter::visit(const Ref<ContinueStmt>& continueStmtNode) {
  dumpNodeInfo(continueStmtNode);
}

void ASTPrinter::visit(const Ref<LabelStmt>& labelStmtNode) {
  dumpNodeInfo(labelStmtNode);
  dump("Name", labelStmtNode->getLabelName());
}

void ASTPrinter::visit(const Ref<GotoStmt>& gotoStmtNode) {
  dumpNodeInfo(gotoStmtNode);
  dump("Target", gotoStmtNode->getTargetLabel());
}
} // namespace bort::ast
