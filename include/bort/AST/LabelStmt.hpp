#pragma once
#include "bort/AST/ASTNode.hpp"

namespace bort::ast {

class LabelStmt final : public Statement {
private:
  explicit LabelStmt(std::string labelName)
      : Statement{ NodeKind::LabelStmt },
        m_LabelName{ std::move(labelName) } {
  }

public:
  [[nodiscard]] auto getLabelName() const -> std::string {
    return m_LabelName;
  }

  friend class ASTRoot;

private:
  std::string m_LabelName;
};

} // namespace bort::ast
