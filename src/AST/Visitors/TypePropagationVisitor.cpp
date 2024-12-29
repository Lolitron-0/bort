#include "bort/AST/Visitors/TypePropagationVisitor.hpp"
#include "bort/CLI/IO.hpp"
#include "bort/Frontend/Type.hpp"
#include <boost/container_hash/hash_fwd.hpp>
#include <frozen/unordered_map.h>

namespace bort::ast {

TypePropagationVisitor::OpResultTypeMap
    TypePropagationVisitor::s_ArtithmeticOpResultTypeMap{};

TypePropagationVisitor::OpPromotionTypeMap
    TypePropagationVisitor::s_ArithmeticOpPromotionTypeMap{};

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
    };
  }
}

void TypePropagationVisitor::visit(const Ref<BinOpExpr>& binopNode) {
  StructureAwareASTVisitor::visit(binopNode);

  auto lhs{ binopNode->getLhs() };
  auto rhs{ binopNode->getRhs() };
  auto lhsTy{ binopNode->getLhs()->getType() };
  auto rhsTy{ binopNode->getRhs()->getType() };

  if (!lhsTy || !rhsTy) {
    // we failed somewhere else, so just skip this node
    return;
  }

  if (binopNode->isArithmetic()) {
    auto opResultTy{
      s_ArtithmeticOpResultTypeMap[{ lhsTy->getKind(), rhsTy->getKind() }]
    };
    if (!opResultTy) {
      Diagnostic::emitError(
          getNodeDebugInfo(binopNode).token,
          "Invalid operands to arithmetic expression: {}, {}",
          lhsTy->toString(), rhsTy->toString());
      return;
    }

    /// @todo probably should annotate promotion instead of changing type
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
  }
}

} // namespace bort::ast
