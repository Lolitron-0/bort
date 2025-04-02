#include "bort/Basic/Assert.hpp"
#include "bort/Basic/VisitDispatcher.hpp"
#include "bort/Codegen/InstructionVisitorBase.hpp"
#include "bort/Codegen/RARSMacroCallInst.hpp"
#include "bort/IR/AllocaInst.hpp"
#include "bort/IR/BranchInst.hpp"
#include "bort/IR/CallInst.hpp"
#include "bort/IR/GepInst.hpp"
#include "bort/IR/Instruction.hpp"
#include "bort/IR/LoadInst.hpp"
#include "bort/IR/MoveInst.hpp"
#include "bort/IR/RetInst.hpp"
#include "bort/IR/StoreInst.hpp"
#include "bort/IR/UnaryInst.hpp"

using namespace bort::ir;

template <>
struct bort::VisitorTraits<bort::codegen::InstructionVisitorBase> {
  static constexpr bool ignoreUnhandled = false;
  static constexpr bool isRefBased      = true;
};

namespace bort::codegen {

void InstructionVisitorBase::genericVisit(
    const Ref<ir::Instruction>& inst) {
  VisitDispatcher<InstructionVisitorBase, Instruction, OpInst, UnaryInst,
                  GepInst, MoveInst, LoadInst, StoreInst, BranchInst,
                  CallInst, RetInst, rv::RARSMacroCallInst,
                  AllocaInst>::dispatch(inst, *this);
  // if (auto opInst{ dynCastRef<OpInst>(inst) }) {
  //   visit(opInst);
  // } else if (auto unaryInst{ dynCastRef<ir::UnaryInst>(inst) }) {
  //   visit(unaryInst);
  // } else if (auto idxInst{ dynCastRef<ir::GepInst>(inst) }) {
  //   visit(idxInst);
  // } else if (auto mvInst{ dynCastRef<MoveInst>(inst) }) {
  //   visit(mvInst);
  // } else if (auto loadInst{ dynCastRef<LoadInst>(inst) }) {
  //   visit(loadInst);
  // } else if (auto storeInst{ dynCastRef<StoreInst>(inst) }) {
  //   visit(storeInst);
  // } else if (auto brInst{ dynCastRef<ir::BranchInst>(inst) }) {
  //   visit(brInst);
  // } else if (auto callInst{ dynCastRef<ir::CallInst>(inst) }) {
  //   visit(callInst);
  // } else if (auto retInst{ dynCastRef<ir::RetInst>(inst) }) {
  //   visit(retInst);
  // } else if (auto macroInst{ dynCastRef<rv::RARSMacroCallInst>(inst) })
  // {
  //   visit(macroInst);
  // } else if (auto allocaInst{ dynCastRef<ir::AllocaInst>(inst) }) {
  //   ///
  // } else {
  //   bort_assert(false,
  //               "Unhandled value in InstructionVisitor generic visit");
  // }
}

void InstructionVisitorBase::run(ir::Module& module) {
  m_CurrentModule = &module;
  for (m_CurrentFuncIter = module.begin();
       m_CurrentFuncIter != module.end(); m_CurrentFuncIter++) {
    for (m_CurrentBBIter = m_CurrentFuncIter->begin();
         m_CurrentBBIter != m_CurrentFuncIter->end(); m_CurrentBBIter++) {
      auto& bb{ *m_CurrentBBIter };
      for (m_CurrentInstIter = bb.begin(); m_CurrentInstIter != bb.end();
           m_CurrentInstIter++) {
        genericVisit(*m_CurrentInstIter);
      }
    }
  }
  m_CurrentModule = nullptr;
}

void InstructionRemover::run(ir::Module& module) {
  for (m_CurrentFuncIter = module.begin();
       m_CurrentFuncIter != module.end(); m_CurrentFuncIter++) {
    for (m_CurrentBBIter = m_CurrentFuncIter->begin();
         m_CurrentBBIter != m_CurrentFuncIter->end(); m_CurrentBBIter++) {
      auto& bb{ *m_CurrentBBIter };
      for (m_CurrentInstIter = bb.begin();
           m_CurrentInstIter != bb.end();) {
        auto iter{ m_CurrentInstIter++ };

        if ((*iter)->getMDNode<RemoveInstructionMDTag>()) {
          bb.removeAt(iter);
        }
      }
    }
  }
}
auto InstructionVisitorBase::getCurrentModule() const -> ir::Module* {
  bort_assert(m_CurrentModule, "Visitor is not being run");
  return m_CurrentModule;
}
} // namespace bort::codegen
