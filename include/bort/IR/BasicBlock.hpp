#pragma once
#include "bort/Basic/Casts.hpp"
#include "bort/IR/Instruction.hpp"
#include "bort/IR/Value.hpp"
#include <list>

namespace bort::ir {

class BranchInst;

using InstList = std::list<Ref<Instruction>>;
using InstIter = InstList::iterator;

class BasicBlock final : public Value {
public:
  explicit BasicBlock(std::string name);

  void addInstruction(Ref<Instruction> instruction) {
    m_Instructions.push_back(std::move(instruction));
  }

  void insertBefore(InstIter pos, Ref<Instruction> instruction) {
    m_Instructions.insert(pos, std::move(instruction));
  }

  void removeAt(InstIter pos) {
    m_Instructions.erase(pos);
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

  [[nodiscard]] auto getLast() const -> Ref<Instruction>;
  [[nodiscard]] auto getLastAsBranch() const -> Ref<BranchInst>;

private:
  InstList m_Instructions;
  static size_t s_NameCounter;
};

} // namespace bort::ir
