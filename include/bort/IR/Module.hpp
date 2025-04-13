#pragma once
#include "bort/Frontend/Symbol.hpp"
#include "bort/Frontend/Type.hpp"
#include "bort/IR/BasicBlock.hpp"
#include "bort/IR/GlobalValue.hpp"
#include "bort/IR/Instruction.hpp"
#include "bort/IR/Value.hpp"
#include <algorithm>
#include <ranges>

namespace bort::ir {

class IRFunction : public Value {
public:
  explicit IRFunction(const Ref<Function>& function);

  auto addBB(std::string name) -> BasicBlock*;

  [[nodiscard]] auto getBB(std::string name) -> BasicBlock*;

  auto erase(std::list<BasicBlock>::iterator it) {
    return m_BasicBlocks.erase(it);
  }

  auto getEntryBlock() -> BasicBlock&;
  auto back() -> BasicBlock&;

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
  Module();

  auto addInstruction(Ref<Instruction> instruction) -> ValueRef;

  void revalidateBasicBlocks();

  auto addGlobal(const Ref<GlobalValue>& global) -> Ref<GlobalValue>;

  [[nodiscard]] auto getGlobals() const {
    return std::views::values(m_Globals);
  }

  [[nodiscard]] auto getGlobalVariable(const std::string& name)
      -> Ref<GlobalVariable>;

  [[nodiscard]] auto getGlobalVariable(const Ref<Variable>& variable)
      -> Ref<GlobalVariable>;

  void addBasicBlock(std::string name);

  void addFunction(Ref<Function> function);

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

  [[nodiscard]] auto getLastFunctionIt() {
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
  std::unordered_map<std::string, Ref<GlobalValue>> m_Globals;
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
