#pragma once
#include "bort/IR/BasicBlock.hpp"
#include "bort/IR/Value.hpp"

namespace bort::ir {

class Module {
public:
  auto addInstruction(Ref<Instruction> instruction) -> ValueRef {
    m_BasicBlocks.back().addInstruction(std::move(instruction));
    return m_BasicBlocks.back().getInstructions().back();
  }

  void addBasicBlock(std::string name) {
    m_BasicBlocks.emplace_back(std::move(name));
  }

  [[nodiscard]] auto getBasicBlocks() const
      -> const std::list<BasicBlock>& {
    return m_BasicBlocks;
  }

  [[nodiscard]] auto begin() {
    return m_BasicBlocks.begin();
  }
  [[nodiscard]] auto end() {
    return m_BasicBlocks.end();
  }
  [[nodiscard]] auto begin() const {
    return m_BasicBlocks.begin();
  }
  [[nodiscard]] auto end() const {
    return m_BasicBlocks.end();
  }

private:
  std::list<BasicBlock> m_BasicBlocks;
};

} // namespace bort::ir
