#include "bort/AST/Visitors/TypePropagationVisitor.hpp"
#include "bort/AST/ASTDebugInfo.hpp"
#include "bort/AST/ASTNode.hpp"
#include "bort/AST/NumberExpr.hpp"
#include "bort/AST/UnaryOpExpr.hpp"
#include "bort/AST/Visitors/ASTVisitor.hpp"
#include "bort/Basic/Casts.hpp"
#include "bort/CLI/IO.hpp"
#include "bort/Frontend/FrontEndInstance.hpp"
#include "bort/Frontend/Type.hpp"
#include "bort/Lex/Token.hpp"
#include <boost/container_hash/hash_fwd.hpp>
#include <frozen/unordered_map.h>

namespace bort::ast {

class FatalSemanticError : public FrontEndFatalError {
public:
  FatalSemanticError()
      : FrontEndFatalError{ "Semantic check failed" } {
  }
};

TypePropagationVisitor::OpResultTypeMap
    TypePropagationVisitor::s_ArtithmeticOpResultTypeMap{};

TypePropagationVisitor::OpPromotionTypeMap
    TypePropagationVisitor::s_ArithmeticOpPromotionTypeMap{};

auto TypePropagationVisitor::getArithmeticOpResultType(
    const TypeRef& lhsType, const TypeRef& rhsType) -> TypeRef {
  if (auto result{ s_ArtithmeticOpResultTypeMap[{
          lhsType->getKind(), rhsType->getKind() }] }) {
    return result;
  }

  TypeRef result{ nullptr };
  auto tryHandlePointerTypes{ [&](const TypeRef& type,
                                  const TypeRef& other) {
    auto ptrType{ dynCastRef<PointerType>(type) };
    if (!ptrType) {
      return;
    }

    if (other->getKind() == TypeKind::Int ||
        other->getKind() == TypeKind::Char) {
      result = ptrType;
    }
  } };

  tryHandlePointerTypes(lhsType, rhsType);
  tryHandlePointerTypes(rhsType, lhsType);
  return result;
}

auto detail::TypePairHasher::operator()(
    const std::pair<TypeKind, TypeKind>& pair) const -> std::size_t {
  std::size_t seed{ 0 };
  boost::hash_combine(seed, pair.first);
  boost::hash_combine(seed, pair.second);
  return seed;
}

template <typename T, typename K, typename V, typename H>
static auto getOr(const std::unordered_map<K, V, H>& map, const K& key,
                  const T& defaultValue) {
  if (map.contains(key)) {
    return map.at(key);
  }
  return V{ defaultValue };
}

TypePropagationVisitor::TypePropagationVisitor() {
  // initializing it here to avoid potential exception in type
  // construction
  // @todo: pointer arithmetic
  if (s_ArtithmeticOpResultTypeMap.empty()) {
    s_ArtithmeticOpResultTypeMap = {
      { { TypeKind::Int, TypeKind::Int }, IntType::get() },
      { { TypeKind::Char, TypeKind::Int }, IntType::get() },
      { { TypeKind::Int, TypeKind::Char }, IntType::get() },
      { { TypeKind::Char, TypeKind::Char }, CharType::get() }
    };

    s_ArithmeticOpPromotionTypeMap = {
      { { TypeKind::Char, TypeKind::Int }, IntType::get() },
      { { TypeKind::Int, TypeKind::Char }, CharType::get() },
    };
  }
}

void TypePropagationVisitor::visit(const Ref<BinOpExpr>& binopNode) {
  StructureAwareASTVisitor::visit(binopNode);

  auto lhs{ binopNode->getLHS() };
  auto rhs{ binopNode->getRHS() };
  auto lhsTy{ binopNode->getLHS()->getType() };
  auto rhsTy{ binopNode->getRHS()->getType() };

  if (!lhsTy || !rhsTy) {
    // we failed somewhere else, so just skip this node
    return;
  }

  if (binopNode->isArithmetic()) {
    auto opResultTy{ getArithmeticOpResultType(lhsTy, rhsTy) };
    if (!opResultTy) {
      Diagnostic::emitError(
          getNodeDebugInfo(binopNode).token,
          "Invalid operands to arithmetic expression: {}, {}",
          lhsTy->toString(), rhsTy->toString());
      throw FatalSemanticError();
    }

    auto lhsPromotedTy{ s_ArithmeticOpPromotionTypeMap[{
        lhsTy->getKind(), opResultTy->getKind() }] };
    if (lhsPromotedTy) {
      lhs->setType(lhsPromotedTy);
    }

    auto rhsPromotedTy{ s_ArithmeticOpPromotionTypeMap[{
        rhsTy->getKind(), opResultTy->getKind() }] };
    if (rhsPromotedTy) {
      rhs->setType(rhsPromotedTy);
    }

    binopNode->setType(opResultTy);
  } else if (binopNode->isLogical()) {
    binopNode->setType(IntType::get());
  } else if (binopNode->getOp() == TokenKind::Assign) {
    assertLvalue(binopNode->getLHS());

    auto lhsTy{ binopNode->getLHS()->getType() };
    auto rhsTy{ binopNode->getRHS()->getType() };

    promoteAssignmentOperands(lhsTy, rhsTy,
                              getASTRoot()->getNodeDebugInfo(binopNode));
    binopNode->setType(lhsTy);
  }
}

void TypePropagationVisitor::visit(const Ref<UnaryOpExpr>& unaryOpNode) {
  StructureAwareASTVisitor::visit(unaryOpNode);
  TypeRef type{ unaryOpNode->getOperand()->getType() };

  switch (unaryOpNode->getOp()) {
  case TokenKind::Amp:
    assertLvalue(unaryOpNode->getOperand());
    // for arrays it's address of first element
    if (auto arrOpType{ dynCastRef<ArrayType>(type) }) {
      type = PointerType::get(arrOpType->getBaseType());
    } else {
      type = PointerType::get(type);
    }
    break;
  case TokenKind::Star:
    if (auto ptrType{ dynCastRef<PointerType>(type) }) {
      type = ptrType->getPointee();
    } else {
      Diagnostic::emitError(
          getASTRoot()->getNodeDebugInfo(unaryOpNode).token,
          "Invalid operand to dereference expression: {}",
          type->toString());
      throw FatalSemanticError();
    }
    break;
  case TokenKind::PlusPlus:
  case TokenKind::MinusMinus:
    assertLvalue(unaryOpNode->getOperand());
    break;
  default:
    break;
  }

  unaryOpNode->setType(std::move(type));
}

void TypePropagationVisitor::visit(
    const Ref<IndexationExpr>& indexationNode) {
  StructureAwareASTVisitor::visit(indexationNode);

  auto arrayTy{ dynCastRef<ArrayType>(
      indexationNode->getArray()->getType()) };
  if (!arrayTy) {
    Diagnostic::emitError(
        getASTRoot()->getNodeDebugInfo(indexationNode->getArray()).token,
        "Expression should have array type, got {} instead",
        indexationNode->getArray()->getType()->toString());
    throw FatalSemanticError();
  }

  indexationNode->setType(arrayTy->getBaseType());
}

void TypePropagationVisitor::visit(const Ref<VarDecl>& varDeclNode) {
  StructureAwareASTVisitor::visit(varDeclNode);
  if (varDeclNode->hasInitializer()) {
    auto varTy{ varDeclNode->getVariable()->getType() };
    auto initTy{ varDeclNode->getInitializer()->getType() };
    promoteAssignmentOperands(
        varTy, initTy, getASTRoot()->getNodeDebugInfo(varDeclNode));
  }
}

void TypePropagationVisitor::promoteAssignmentOperands(
    const TypeRef& lhsTy, TypeRef& rhsTy, const ASTDebugInfo& debugInfo) {
  auto rhsPromotedTy{
    s_ArithmeticOpPromotionTypeMap[{ rhsTy->getKind(), lhsTy->getKind() }]
  };

  if (isaRef<IntType>(rhsTy) && isaRef<CharType>(lhsTy)) {
    Diagnostic::emitWarning(debugInfo.token,
                            "Narrowing conversion from {} to {}",
                            rhsTy->toString(), lhsTy->toString());
  }

  if (rhsPromotedTy) {
    rhsTy = rhsPromotedTy;
  }

  if (rhsTy != lhsTy) {
    Diagnostic::emitError(
        debugInfo.token,
        "Invalid assignment RHS type, expected {}, got {} instead",
        lhsTy->toString(), rhsTy->toString());
    throw FatalSemanticError();
  }
}

void TypePropagationVisitor::assertLvalue(const Ref<Node>& node) {
  if (node->getKind() == NodeKind::VariableExpr ||
      node->getKind() == NodeKind::IndexationExpr) {
    return;
  }

  if (auto unOpNode{ dynCastRef<UnaryOpExpr>(node) }) {
    if (unOpNode->getOp() == TokenKind::Star) {
      return;
    }
  }

  Diagnostic::emitError(getASTRoot()->getNodeDebugInfo(node).token,
                        "Expected lvalue");
  throw FatalSemanticError();
}

} // namespace bort::ast
