#pragma once
#include "bort/IR/BasicBlock.hpp"
#include "bort/IR/Value.hpp"

namespace bort::ir {

using BBIter = std::list<BasicBlock>::iterator;

class IRFunction : public Value {
public:
  IRFunction(TypeRef type, std::string name)
      : Value{ std::move(type), std::move(name) } {
  }

  void addBB(std::string name) {
    m_BasicBlocks.emplace_back(std::move(name));
  }

  auto erase(std::list<BasicBlock>::iterator it) {
    return m_BasicBlocks.erase(it);
  }

  auto getEntryBlock() -> BasicBlock& {
    return m_BasicBlocks.front();
  }
  auto back() -> BasicBlock& {
    return m_BasicBlocks.back();
  }

  [[nodiscard]] auto begin() {
    return m_BasicBlocks.begin();
  }
  [[nodiscard]] auto end() {
    return m_BasicBlocks.end();
  }

  [[nodiscard]] auto begin() const {
    return m_BasicBlocks.cbegin();
  }
  [[nodiscard]] auto end() const {
    return m_BasicBlocks.cend();
  }

private:
  std::list<BasicBlock> m_BasicBlocks;
};

class Module {
public:
  auto addInstruction(Ref<Instruction> instruction) -> ValueRef {
    auto& lastBB{ m_Functions.back().back() };
    lastBB.addInstruction(std::move(instruction));
    return lastBB.getInstructions().back();
  }

  void revalidateBasicBlocks() {
    for (auto&& func : m_Functions) {
      for (auto it{ func.begin() }; it != func.end();) {
        /// last BB can be empty
        if (it->getInstructions().empty() &&
            ++decltype(it){ it } != func.end()) {
          it = func.erase(it);
        } else {
          it++;
        }
      }
    }
  }

  void addBasicBlock(std::string name) {
    m_Functions.back().addBB(std::move(name));
  }

  void addFunction(TypeRef type, std::string name) {
    m_Functions.emplace_back(std::move(type), std::move(name));
  }

  [[nodiscard]] auto begin() {
    return m_Functions.begin();
  }
  [[nodiscard]] auto end() {
    return m_Functions.end();
  }
  [[nodiscard]] auto begin() const {
    return m_Functions.begin();
  }
  [[nodiscard]] auto end() const {
    return m_Functions.end();
  }
  [[nodiscard]] auto getLastBBIt() const {
    auto it{ m_Functions.back().end() };
    it--;
    return it;
  }

  [[nodiscard]] auto getLastBBIt() {
    auto it{ m_Functions.back().end() };
    it--;
    return it;
  }

private:
  std::list<IRFunction> m_Functions;
};

} // namespace bort::ir
