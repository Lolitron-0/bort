#pragma once
#include "bort/AST/BinOpExpr.hpp"
#include "bort/AST/IndexationExpr.hpp"
#include "bort/AST/UnaryOpExpr.hpp"
#include "bort/AST/VarDecl.hpp"
#include "bort/AST/Visitors/ASTVisitor.hpp"
#include "bort/Frontend/Type.hpp"
#include <unordered_map>

namespace bort::ast {

namespace detail {
struct TypePairHasher {
  auto operator()(const std::pair<TypeKind, TypeKind>& pair) const
      -> std::size_t;
};
} // namespace detail

class TypePropagationVisitor : public StructureAwareASTVisitor {
public:
  using OpResultTypeMap =
      std::unordered_map<std::pair<TypeKind, TypeKind>, TypeRef,
                         detail::TypePairHasher>;
  using OpPromotionTypeMap =
      std::unordered_map<std::pair<TypeKind, TypeKind>, TypeRef,
                         detail::TypePairHasher>;
  TypePropagationVisitor();

private:
  void visit(const Ref<BinOpExpr>& binopNode) override;
  void visit(const Ref<UnaryOpExpr>& unaryOpNode) override;
  void visit(const Ref<IndexationExpr>& indexationNode) override;
  void visit(const Ref<VarDecl>& varDeclNode) override;

  [[nodiscard]] static auto getArithmeticOpResultType(
      const TypeRef& lhsType, const TypeRef& rhsType) -> TypeRef;

  static OpResultTypeMap s_ArtithmeticOpResultTypeMap;
  /// for pair {t1, t2}, shows how t1 needs to be promoted when op result
  /// is t2
  static OpPromotionTypeMap s_ArithmeticOpPromotionTypeMap;
};

} // namespace bort::ast
