#pragma once
#include "bort/AST/ASTNode.hpp"

namespace bort::ast {

class Block final : public Statement {
  Block();

public:
  [[nodiscard]] auto getBody() const -> const std::vector<Ref<Node>>&;
  void pushChild(Ref<Node> child);

  friend class ASTRoot;

private:
  std::vector<Ref<Node>> m_Body;
};

} // namespace bort::ast
