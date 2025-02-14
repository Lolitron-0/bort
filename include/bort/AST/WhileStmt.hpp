#pragma once
#include "bort/AST/ASTNode.hpp"
#include "bort/AST/Block.hpp"
#include "bort/AST/ExpressionNode.hpp"

namespace bort::ast {

class WhileStmt : public Statement {
public:
  WhileStmt(Ref<ExpressionNode> condition, Ref<Block> body)
      : Statement{ NodeKind::WhileStmt },
        m_Condition{ std::move(condition) },
        m_Body{ std::move(body) } {
  }

  [[nodiscard]] auto getCondition() const -> Ref<ExpressionNode> {
    return m_Condition;
  }
  [[nodiscard]] auto getBody() const -> Ref<Block> {
    return m_Body;
  }

private:
  Ref<ExpressionNode> m_Condition;
  Ref<Block> m_Body;
};

} // namespace bort::ast
