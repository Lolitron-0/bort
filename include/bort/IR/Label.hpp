#pragma once
#include "bort/Basic/Assert.hpp"
#include "bort/IR/BasicBlock.hpp"

namespace bort::ir {

class Label {
public:
  explicit Label(std::string name = "");

  void setBlock(const BasicBlock* block) {
    m_Block = block;
  }

  [[nodiscard]] auto getBlock() -> const BasicBlock* {
    bort_assert(m_Block, "Block not set");
    return m_Block;
  }

  [[nodiscard]] auto getName() const -> std::string {
    return m_Name;
  }

private:
  std::string m_Name;
  const BasicBlock* m_Block{ nullptr };
  static size_t s_NameCounter;
};

} // namespace bort::ir
