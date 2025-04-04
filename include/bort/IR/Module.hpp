#pragma once
#include "bort/Frontend/Symbol.hpp"
#include "bort/Frontend/Type.hpp"
#include "bort/IR/BasicBlock.hpp"
#include "bort/IR/GlobalValue.hpp"
#include "bort/IR/Instruction.hpp"
#include "bort/IR/Value.hpp"
#include <algorithm>

namespace bort::ir {

class IRFunction : public Value {
public:
  explicit IRFunction(const Ref<Function>& function)
      : Value{ VoidType::get(), function->getName() },
        m_Function{ function } {
  }

  auto addBB(std::string name) -> BasicBlock* {
    m_BasicBlocks.emplace_back(std::move(name));
    return &m_BasicBlocks.back();
  }

  [[nodiscard]] auto getBB(std::string name) -> BasicBlock* {
    auto it{ std::find_if(m_BasicBlocks.begin(), m_BasicBlocks.end(),
                          [&name](const auto& bb) {
                            return bb.getName() == name;
                          }) };
    if (it == m_BasicBlocks.end()) {
      return nullptr;
    }
    return &*it;
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

  [[nodiscard]] auto getFunction() const -> Ref<Function> {
    return m_Function;
  }

private:
  std::list<BasicBlock> m_BasicBlocks;
  Ref<Function> m_Function;
};

using BBIter     = std::list<BasicBlock>::iterator;
using IRFuncIter = std::list<IRFunction>::iterator;

class Module : public Value {
public:
  Module()
      : Value{ VoidType::get(), "module" } {
  }

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

  auto addGlobal(Ref<GlobalValue> global) -> Ref<GlobalValue> {
    m_Globals.push_back(std::move(global));
    return m_Globals.back();
  }

  [[nodiscard]] auto getGlobals() const
      -> const std::vector<Ref<GlobalValue>>& {
    return m_Globals;
  }

  void addBasicBlock(std::string name) {
    m_Functions.back().addBB(std::move(name));
  }

  void addFunction(Ref<Function> function) {
    m_Functions.emplace_back(std::move(function));
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

  [[nodiscard]] auto getLastFunctionIt() const {
    auto it{ m_Functions.end() };
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
  std::vector<Ref<GlobalValue>> m_Globals;
};

struct TraversalContext {
  IRFuncIter funcIt;
  BBIter bbIt;
  InstIter instIt;

  void insertInstruction(Ref<Instruction> instruction) const {
    return bbIt->insertBefore(instIt, std::move(instruction));
  }
};

} // namespace bort::ir
