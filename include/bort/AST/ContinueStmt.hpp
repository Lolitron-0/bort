#pragma once
#include "bort/AST/ASTNode.hpp"

namespace bort::ast {

class ContinueStmt final : public Statement {
private:
  ContinueStmt()
      : Statement{ NodeKind::ContinueStmt } {
  }

public:
  friend class ASTRoot;
};

} // namespace bort::ast
