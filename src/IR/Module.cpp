#include "bort/IR/Module.hpp"
#include "bort/Basic/Assert.hpp"
#include "bort/IR/BranchInst.hpp"
#include "bort/IR/GlobalValue.hpp"

using namespace bort::ir;
using namespace bort;

IRFunction::IRFunction(const Ref<Function>& function)
    : Value{ VoidType::get(), function->getName() },
      m_Function{ function } {
}

auto IRFunction::addBB(std::string name) -> BasicBlock* {
  m_BasicBlocks.emplace_back(std::move(name));
  return &m_BasicBlocks.back();
}

auto IRFunction::getBB(std::string name) -> BasicBlock* {
  auto it{ std::find_if(m_BasicBlocks.begin(), m_BasicBlocks.end(),
                        [&name](const auto& bb) {
                          return bb.getName() == name;
                        }) };
  if (it == m_BasicBlocks.end()) {
    return nullptr;
  }
  return &*it;
}

auto IRFunction::getEntryBlock() -> BasicBlock& {
  return m_BasicBlocks.front();
}

auto IRFunction::back() -> BasicBlock& {
  return m_BasicBlocks.back();
}

Module::Module()
    : Value{ VoidType::get(), "module" } {
}

auto bort::ir::Module::addInstruction(Ref<Instruction> instruction)
    -> ValueRef {
  auto& lastBB{ m_Functions.back().back() };
  lastBB.addInstruction(std::move(instruction));
  return lastBB.getInstructions().back();
}

static auto hasReferencingBranch(const IRFunction& func,
                                 BasicBlock* bb) -> bool {
  for (auto&& BB : func) {
    for (auto&& inst : BB) {
      if (auto br{ dynCastRef<BranchInst>(inst) }) {
        if (br->getTarget() == bb) {
          return true;
        }
      }
    }
  }
  return false;
}

void bort::ir::Module::revalidateBasicBlocks() {
  for (auto&& func : m_Functions) {
    for (auto it{ func.begin() }; it != func.end();) {
      /// last BB can be empty
      if (it->getInstructions().empty() &&
          ++decltype(it){ it } != func.end() &&
          !hasReferencingBranch(func, &*it)) {
        it = func.erase(it);
      } else {
        it++;
      }
    }
  }
}

auto bort::ir::Module::addGlobal(const Ref<GlobalValue>& global)
    -> Ref<GlobalValue> {
  m_Globals[global->getName()] = global;
  return m_Globals.at(global->getName());
}

auto bort::ir::Module::getGlobalVariable(const std::string& name)
    -> Ref<GlobalVariable> {
  bort_assert(m_Globals.contains(name), "No GlobalValue with such name");
  auto GV{ dynCastRef<GlobalVariable>(m_Globals.at(name)) };
  bort_assert(GV,
              "GlobalValue with requested name is not a GlobalVariable");
  return GV;
}

auto bort::ir::Module::getGlobalVariable(const Ref<Variable>& variable)
    -> Ref<GlobalVariable> {
  return getGlobalVariable(variable->getName());
}

void bort::ir::Module::addBasicBlock(std::string name) {
  m_Functions.back().addBB(std::move(name));
}

void bort::ir::Module::addFunction(Ref<Function> function) {
  m_Functions.emplace_back(std::move(function));
}
