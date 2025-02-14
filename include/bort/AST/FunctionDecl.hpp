#pragma once
#include "bort/AST/ASTNode.hpp"
#include "bort/AST/Block.hpp"
#include "bort/Frontend/Symbol.hpp"

namespace bort::ast {

class FunctionDecl : public Statement {
  FunctionDecl(std::string name, TypeRef returnType,
               std::vector<Ref<Variable>> args, Ref<Block> body)
      : Statement{ NodeKind::FunctionDecl },
        m_FunctionSymbol{ makeRef<Function>(
            std::move(name), std::move(returnType), args) },
        m_Body{ std::move(body) } {
  }

public:
  [[nodiscard]] auto getFunction() const -> Ref<Function> {
    return m_FunctionSymbol;
  }
  [[nodiscard]] auto getBody() const -> Ref<Block> {
    return m_Body;
  }

  friend class ASTRoot;

private:
  Ref<Function> m_FunctionSymbol;
  Ref<Block> m_Body;
};

} // namespace bort::ast
