#pragma once
#include "bort/IR/Instruction.hpp"
#include "bort/IR/Value.hpp"
#include <list>

namespace bort::ir {

using InstList = std::list<Ref<Instruction>>;
using InstIter = InstList::iterator;

class BasicBlock final : public Value {
public:
  explicit BasicBlock(std::string name);

  void addInstruction(Ref<Instruction> instruction) {
    m_Instructions.push_back(std::move(instruction));
  }

  void insertBefore(InstList::iterator pos,
                    Ref<Instruction> instruction) {
    m_Instructions.insert(pos, std::move(instruction));
  }

  [[nodiscard]] auto getInstructions() const -> const InstList& {
    return m_Instructions;
  }

  [[nodiscard]] auto begin() {
    return m_Instructions.begin();
  }
  [[nodiscard]] auto end() {
    return m_Instructions.end();
  }

  [[nodiscard]] auto begin() const {
    return m_Instructions.begin();
  }
  [[nodiscard]] auto end() const {
    return m_Instructions.end();
  }

private:
  InstList m_Instructions;
  static size_t s_NameCounter;
};

} // namespace bort::ir
