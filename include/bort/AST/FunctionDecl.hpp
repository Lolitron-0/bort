#pragma once
#include "bort/AST/ASTNode.hpp"
#include "bort/Frontend/Symbol.hpp"

namespace bort::ast {

class FunctionDecl : public Node {
  FunctionDecl(std::string name, TypeRef returnType,
               std::vector<Variable> args, Ref<Block> body)
      : Node{ NodeKind::FunctionDecl },
        m_FunctionSymbol{ makeRef<Function>(
            std::move(name), std::move(returnType), args) },
        m_Body{ std::move(body) } {
  }

public:
  [[nodiscard]] auto getBody() const -> Ref<Block> {
    return m_Body;
  }

  friend class ASTRoot;

private:
  Ref<Function> m_FunctionSymbol;
  Ref<Block> m_Body;
};

} // namespace bort::ast
