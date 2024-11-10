#include "bort/AST/Block.hpp"

namespace bort::ast {

Block::Block()
    : Node{ NodeKind::Block } {
}

auto Block::getBody() const -> const std::vector<Ref<Node>>& {
  return m_Body;
}

void Block::pushChild(Ref<Node> child) {
  m_Body.push_back(std::move(child));
}

} // namespace bort::ast
