#pragma once
#include "bort/AST/ASTNode.hpp"

namespace bort::ast {

class BreakStmt final : public Statement {
private:
  BreakStmt()
      : Statement{ NodeKind::BreakStmt } {
  }

public:
  friend class ASTRoot;
};

} // namespace bort::ast
