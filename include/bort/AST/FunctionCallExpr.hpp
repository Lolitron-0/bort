#pragma once
#include "bort/AST/ASTNode.hpp"
#include "bort/AST/ExpressionNode.hpp"
#include "bort/Frontend/Symbol.hpp"

namespace bort::ast {

class FunctionCallExpr final : public ExpressionNode {
public:
  FunctionCallExpr(Ref<Function> function,
                   std::vector<Ref<ExpressionNode>> args)
      : ExpressionNode{ NodeKind::FunctionCallExpr },
        m_Args{ std::move(args) } {
    setFunction(std::move(function));
  }

  [[nodiscard]] auto getFunction() const -> Ref<Function> {
    return m_Function;
  }

  [[nodiscard]] auto isResolved() const -> bool {
    return !m_Function->isShallow();
  }

  void setFunction(Ref<Function> function) {
    m_Function = std::move(function);
    setType(m_Function->isShallow() ? nullptr
                                    : m_Function->getReturnType());
  }

  [[nodiscard]] auto getArgs() const
      -> const std::vector<Ref<ExpressionNode>>& {
    return m_Args;
  }

private:
  Ref<Function> m_Function;
  std::vector<Ref<ExpressionNode>> m_Args;
};

} // namespace bort::ast
