#pragma once
#include "bort/AST/ASTNode.hpp"

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

  void preOrderVisit(const Ref<ASTVisitor>& visitor) override;
  void postOrderVisit(const Ref<ASTVisitor>& visitor) override;

protected:
  void dump(int depth) const override;

private:
  Ref<Variable> m_Variable;
};

} // namespace bort::ast
