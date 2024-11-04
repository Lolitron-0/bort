#pragma once
#include "bort/AST/ASTNode.hpp"

namespace bort::ast {

class Block final : public Node {
  Block()
      : Node{ NodeKind::Block } {
  }

public:
  [[nodiscard]] auto getBody() const -> const std::vector<Ref<Node>>& {
    return m_Body;
  }
  void pushChild(Ref<Node> child) {
    m_Body.push_back(std::move(child));
  }

  void preOrderVisit(const Ref<ASTVisitor>& visitor) override;
  void postOrderVisit(const Ref<ASTVisitor>& visitor) override;

  friend class ASTRoot;

private:
  std::vector<Ref<Node>> m_Body;
};

} // namespace bort::ast
