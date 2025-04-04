#pragma once
#include "bort/AST/ExpressionNode.hpp"
#include "bort/AST/NumberExpr.hpp"
#include "bort/Frontend/Type.hpp"
#include <utility>

namespace bort::ast {

class InitializerList final : public ExpressionNode {
public:
  explicit InitializerList(std::vector<Ref<NumberExpr>> values)
      : ExpressionNode{ NodeKind::InitializerList,
                        ArrayType::get(values.front()->getType(),
                                       values.size()) },
        m_Values{ std::move(values) } {
  }

  [[nodiscard]] auto getValues() const
      -> const std::vector<Ref<NumberExpr>>& {
    return m_Values;
  }

private:
  std::vector<Ref<NumberExpr>> m_Values;
};

} // namespace bort::ast
