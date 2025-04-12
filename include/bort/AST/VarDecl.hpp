#pragma once
#include <utility>

#include "bort/AST/ASTNode.hpp"
#include "bort/AST/ExpressionNode.hpp"
#include "bort/Basic/Assert.hpp"
#include "bort/Frontend/Symbol.hpp"

namespace bort::ast {

class VarDecl final : public Statement {

  VarDecl(TypeRef type, std::string name, bool global = false)
      : Statement{ NodeKind::VarDecl },
        m_Variable{ makeRef<Variable>(std::move(name), std::move(type),
                                      global) } {
  }

public:
  friend class ASTRoot;

  [[nodiscard]] auto getVariable() -> Ref<Variable> {
    return m_Variable;
  }

  [[nodiscard]] auto hasInitializer() const -> bool {
    return m_Initializer != nullptr;
  }

  void setInitializer(Ref<ExpressionNode> initializer) {
    m_Initializer = std::move(initializer);
  }

  [[nodiscard]] auto getInitializer() -> Ref<ExpressionNode> {
    bort_assert(hasInitializer(),
                "getInitializer on non-initialized var decl");
    return m_Initializer;
  }

private:
  Ref<Variable> m_Variable;
  Ref<ExpressionNode> m_Initializer;
};

} // namespace bort::ast
