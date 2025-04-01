#pragma once
#include "bort/AST/ASTNode.hpp"
#include "bort/AST/ExpressionNode.hpp"
#include "bort/AST/VariableExpr.hpp"

namespace bort::ast {

class IndexationExpr final : public ExpressionNode {
  IndexationExpr(Ref<VariableExpr> arr, Ref<ExpressionNode> index)
      : ExpressionNode{ NodeKind::IndexationExpr },
        m_Array{ std::move(arr) },
        m_Index{ std::move(index) } {
  }

public:
  [[nodiscard]] auto getArray() const -> Ref<VariableExpr> {
    return m_Array;
  }
  [[nodiscard]] auto getIndex() const -> Ref<ExpressionNode> {
    return m_Index;
  }

  friend class ASTRoot;

private:
  Ref<VariableExpr> m_Array;
  Ref<ExpressionNode> m_Index;
};

} // namespace bort::ast
