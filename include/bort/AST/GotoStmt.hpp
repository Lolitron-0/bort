#pragma once
#include "bort/AST/ASTNode.hpp"

namespace bort::ast {

class GotoStmt final : public Statement {
private:
  explicit GotoStmt(std::string targetLabelName)
      : Statement{ NodeKind::GotoStmt },
        m_TargetLabel{ std::move(targetLabelName) } {
  }

public:
  [[nodiscard]] auto getTargetLabel() const -> std::string {
    return m_TargetLabel;
  }

  friend class ASTRoot;

private:
  std::string m_TargetLabel;
};

} // namespace bort::ast
