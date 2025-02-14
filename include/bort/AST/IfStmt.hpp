#pragma once
#include "bort/AST/ASTNode.hpp"
#include "bort/AST/Block.hpp"
#include "bort/AST/ExpressionNode.hpp"

namespace bort::ast {

class IfStmt : public Statement {
public:
  IfStmt(Ref<ExpressionNode> condition, Ref<Block> thenBlock,
             Ref<Block> elseBlock)
      : Statement{ NodeKind::IfStmt },
        m_Condition{ std::move(condition) },
        m_ThenBlock{ std::move(thenBlock) },
        m_ElseBlock{ std::move(elseBlock) } {
  }

  [[nodiscard]] auto getCondition() const -> Ref<ExpressionNode> {
    return m_Condition;
  }
  [[nodiscard]] auto getThenBlock() const -> Ref<Block> {
    return m_ThenBlock;
  }
  [[nodiscard]] auto getElseBlock() const -> Ref<Block> {
    return m_ElseBlock;
  }

private:
  Ref<ExpressionNode> m_Condition;
  Ref<Block> m_ThenBlock;
  Ref<Block> m_ElseBlock;
};

} // namespace bort::ast
