#include "bort/IR/IRCodegen.hpp"
#include "bort/AST/BinOpExpr.hpp"
#include "bort/AST/ExpressionNode.hpp"
#include "bort/AST/Visitors/Utils.hpp"
#include "bort/Basic/Assert.hpp"
#include "bort/Basic/Casts.hpp"
#include "bort/Basic/Ref.hpp"
#include "bort/CLI/IO.hpp"
#include "bort/Frontend/Type.hpp"
#include "bort/IR/AllocaInst.hpp"
#include "bort/IR/BranchInst.hpp"
#include "bort/IR/CallInst.hpp"
#include "bort/IR/Constant.hpp"
#include "bort/IR/GepInst.hpp"
#include "bort/IR/GlobalValue.hpp"
#include "bort/IR/LoadInst.hpp"
#include "bort/IR/Metadata.hpp"
#include "bort/IR/MoveInst.hpp"
#include "bort/IR/OpInst.hpp"
#include "bort/IR/Register.hpp"
#include "bort/IR/RetInst.hpp"
#include "bort/IR/StoreInst.hpp"
#include "bort/IR/UnaryInst.hpp"
#include "bort/IR/Value.hpp"
#include "bort/IR/VariableUse.hpp"
#include "bort/Lex/Token.hpp"
#include <algorithm>
#include <boost/range/adaptor/indexed.hpp>
#include <cul/BiMap.hpp>
#include <fmt/format.h>
#include <utility>

namespace bort::ir {

class StoreSync : public Metadata {
public:
  explicit StoreSync(ValueRef loc)
      : m_Loc{ std::move(loc) } {
  }

  [[nodiscard]] auto toString() const -> std::string override {
    return fmt::format("store_sync .loc=%{}", m_Loc->getName());
  }

  [[nodiscard]] auto getLoc() const -> ValueRef {
    return m_Loc;
  }

private:
  ValueRef m_Loc;
};

auto IRCodegen::genBranchFromCondition(
    const Ref<ast::ExpressionNode>& cond,
    bool negate) -> Ref<BranchInst> {
  static constexpr cul::BiMap s_NegationTable{ [](auto&& selector) {
    return selector.Case(TokenKind::Equals, TokenKind::NotEquals)
        .Case(TokenKind::Greater, TokenKind::LessEqual)
        .Case(TokenKind::Less, TokenKind::GreaterEqual);
  } };

  ValueRef lhs{ nullptr };
  ValueRef rhs{ nullptr };
  TokenKind mode{ TokenKind::NotEquals };

  if (auto binOpCond{ dynCastRef<ast::BinOpExpr>(cond) }) {
    switch (binOpCond->getOp()) {
    case TokenKind::Equals:
    case TokenKind::NotEquals:
    case TokenKind::Greater:
    case TokenKind::GreaterEqual:
    case TokenKind::Less:
    case TokenKind::LessEqual:
      mode = binOpCond->getOp();
      lhs  = genericVisit(binOpCond->getLHS());
      rhs  = genericVisit(binOpCond->getRHS());
      break;
    default:
      break;
    }
  }
  if (!lhs && !rhs) {
    lhs = genericVisit(cond);
    rhs = IntegralConstant::getInt(0);
  }

  if (negate) {
    auto negModeOpt{ s_NegationTable.FindByFirst(mode) };
    if (!negModeOpt) {
      negModeOpt = s_NegationTable.FindBySecond(mode);
    }
    bort_assert(negModeOpt, "Negation table is incomplete");
    mode = *negModeOpt;
  }

  return makeRef<BranchInst>(std::move(lhs), std::move(rhs), mode);
}

auto IRCodegen::genArrayPtr(const ValueRef& arr)
    -> std::pair<Ref<PointerType>, ValueRef> {
  auto arrTy{ dynCastRef<ArrayType>(arr->getType()) };
  bort_assert(arrTy, "Array expected");
  auto ptrTy{ PointerType::get(arrTy->getBaseType()) };

  auto arrPtr{ addInstruction(
                   makeRef<UnaryInst>(TokenKind::Amp,
                                      Register::getOrCreate(ptrTy), arr))
                   ->getDestination() };
  return { ptrTy, arrPtr };
}

void IRCodegen::codegen(const Ref<ast::ASTRoot>& ast) {
  m_ASTRoot = ast;
  genericVisit(ast);
  m_Module.revalidateBasicBlocks();
}

auto IRCodegen::genericVisit(const Ref<ast::Node>& node) -> ValueRef {
  return ast::callHandler(node, [this](const auto& node) {
    return visit(node);
  });
}

auto IRCodegen::visit(const Ref<ast::BinOpExpr>& binOpNode) -> ValueRef {
  auto lhs{ genericVisit(binOpNode->getLHS()) };
  auto rhs{ genericVisit(binOpNode->getRHS()) };

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

auto IRCodegen::visit(const Ref<ast::UnaryOpExpr>& unaryOpExpr)
    -> ValueRef {
  auto operand{ genericVisit(unaryOpExpr->getOperand()) };
  auto dst{ Register::getOrCreate(unaryOpExpr->getType()) };
  Ref<Instruction> result;

  switch (unaryOpExpr->getOp()) {
  case TokenKind::Star:
    result = addInstruction(
        makeRef<LoadInst>(dst, operand,
                          IntegralConstant::getInt(static_cast<int32_t>(
                              operand->getType()->getSizeof()))));
    result->getDestination()->addMDNode(StoreSync{ operand });
    break;
  case TokenKind::Plus:
    result = addInstruction(makeRef<MoveInst>(dst, operand));
    break;
  default:
    result = addInstruction(
        makeRef<UnaryInst>(unaryOpExpr->getOp(), dst, operand));
    break;
  }
  return result->getDestination();
}

auto IRCodegen::visit(const Ref<ast::NumberExpr>& numberNode)
    -> ValueRef {
  return IntegralConstant::getInt(numberNode->getValue());
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
  auto elementSize{ IntegralConstant::getInt(
      static_cast<int32_t>(varSymbol->getType()->getSizeof())) };
  ValueRef numElements{ IntegralConstant::getInt(1) };
  if (auto arrayType{ dynCastRef<ArrayType>(varSymbol->getType()) }) {
    elementSize =
        IntegralConstant::getInt(arrayType->getBaseType()->getSizeof());
    numElements = IntegralConstant::getInt(arrayType->getNumElements());
  }
  auto newVar{ addInstruction(makeRef<AllocaInst>(std::move(varSymbol),
                                                  elementSize,
                                                  numElements))
                   ->getDestination() };

  if (!varDeclNode->hasInitializer()) {
    return newVar;
  }

  auto init{ genericVisit(varDeclNode->getInitializer()) };
  auto move{ addInstruction(makeRef<MoveInst>(newVar, init)) };
  return move->getDestination();
}

auto IRCodegen::visit(
    const Ref<ast::InitializerList>& initializerListNode) -> ValueRef {
  std::vector<Ref<IntegralConstant>> values{};
  for (auto&& num : initializerListNode->getValues()) {
    values.push_back(
        IntegralConstant::getOrCreate(num->getValue(), num->getType()));
  }
  return m_Module.addGlobal(GlobalArray::getOrCreate(std::move(values)));
}

auto IRCodegen::visit(const Ref<ast::IndexationExpr>& indexationNode)
    -> ValueRef {
  auto arr{ genericVisit(indexationNode->getArray()) };

  auto index{ genericVisit(indexationNode->getIndex()) };

  auto [_, arrPtr]{ genArrayPtr(arr) };
  auto gepInst{ addInstruction(
      makeRef<GepInst>(arrPtr, arrPtr, std::move(index))) };

  auto baseTy{ indexationNode->getType() };
  auto deref{ addInstruction(makeRef<LoadInst>(
      Register::getOrCreate(baseTy), gepInst->getDestination(),
      IntegralConstant::getInt(baseTy->getSizeof()))) };

  deref->getDestination()->addMDNode(
      StoreSync{ gepInst->getDestination() });
  return deref->getDestination();
}

auto IRCodegen::visit(const Ref<ast::FunctionDecl>& functionDeclNode)
    -> ValueRef {
  for (auto&& paramVar : functionDeclNode->getFunction()->getArgs()) {
    VariableUse::createUnique(paramVar);
  }
  m_Module.addFunction(functionDeclNode->getFunction());
  pushBB("", functionDeclNode->getFunction()->getName());
  genericVisit(functionDeclNode->getBody());

  bool seenRet{ std::count_if(m_Module.getLastBBIt()->begin(),
                              m_Module.getLastBBIt()->end(),
                              [](const auto& inst) {
                                return isaRef<RetInst>(inst);
                              }) > 0 };
  bool voidFunc{ m_Module.getLastFunctionIt()->getType() ==
                 VoidType::get() };

  if (!voidFunc && !seenRet) {
    Diagnostic::emitWarning(
        m_ASTRoot->getNodeDebugInfo(functionDeclNode).token,
        "Non-void function does not return a value");
  }

  // well, we need to check it somewhere...
  if (voidFunc && !seenRet &&
      functionDeclNode->getFunction()->getName() != "main") {
    addInstruction(makeRef<RetInst>());
  }

  return nullptr;
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
auto IRCodegen::visit(const Ref<ast::IfStmt>& ifStmtNode) -> ValueRef {

  auto thenBr{ addInstruction(
      genBranchFromCondition(ifStmtNode->getCondition())) };
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

auto IRCodegen::visit(const Ref<ast::WhileStmt>& whileStmtNode)
    -> ValueRef {
  pushBB("_cond");
  auto& condBB{ *m_Module.getLastBBIt() };

  auto endBr{ addInstruction(
      genBranchFromCondition(whileStmtNode->getCondition(), true)) };

  pushBB("_body");
  genericVisit(whileStmtNode->getBody());
  auto loopBr{ addInstruction(makeRef<BranchInst>()) };
  loopBr->setTarget(&condBB);

  pushBB("_end");
  endBr->setTarget(&*m_Module.getLastBBIt());
  return nullptr;
}

auto IRCodegen::visit(const Ref<ast::FunctionCallExpr>& funcCallExpr)
    -> ValueRef {
  std::vector<ValueRef> args;
  for (const auto& argExpr : funcCallExpr->getArgs()) {
    args.push_back(genericVisit(argExpr));
  }
  auto function{ funcCallExpr->getFunction() };
  if (function->getReturnType()->is(TypeKind::Void)) {
    addInstruction(makeRef<CallInst>(function, std::move(args)));
    return nullptr;
  }
  auto callInst{ addInstruction(makeRef<CallInst>(
      function, Register::getOrCreate(function->getReturnType()),
      std::move(args))) };
  return callInst->getDestination();
}

auto IRCodegen::visit(const Ref<ast::ReturnStmt>& returnStmt)
    -> ValueRef {
  ValueRef returnValue{ nullptr };
  if (returnStmt->hasExpression()) {
    returnValue = genericVisit(returnStmt->getExpression());
  }
  auto retInst{ addInstruction(makeRef<RetInst>(returnValue)) };
  return nullptr;
}

void IRCodegen::processNewInst(const Ref<Instruction>& instruction) {
  if (instruction->getType() != VoidType::get()) {
    if (auto* SS{
            instruction->getDestination()->getMDNode<StoreSync>() }) {

      auto dest{ instruction->getDestination() };
      // has no destination, so won't recurse
      addInstruction(makeRef<StoreInst>(
          dest, SS->getLoc(),
          IntegralConstant::getInt(dest->getType()->getSizeof())));
    }
  }
}

} // namespace bort::ir
