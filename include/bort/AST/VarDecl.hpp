#pragma once
#include "bort/AST/ASTNode.hpp"
#include "bort/Frontend/Symbol.hpp"

namespace bort::ast {

class VarDecl final : public Node {
  VarDecl(TypeRef type, std::string name)
      : Node{ NodeKind::VarDecl },
        m_Variable{ makeRef<Variable>(std::move(name),
                                      std::move(type)) } {
  }

public:
  friend class ASTRoot;

  [[nodiscard]] auto getVariable() -> Ref<Variable> {
    return m_Variable;
  }

private:
  Ref<Variable> m_Variable;
};

} // namespace bort::ast
