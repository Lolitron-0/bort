#include "bort/Basic/Assert.hpp"
#include "bort/Codegen/InstructionVisitorBase.hpp"
#include "bort/IR/AllocaInst.hpp"
#include "bort/IR/CallInst.hpp"
#include "bort/IR/RetInst.hpp"
#include "bort/IR/StoreInst.hpp"
#include "bort/IR/UnaryInst.hpp"

using namespace bort::ir;

namespace bort::codegen {

void InstructionVisitorBase::genericVisit(
    const Ref<ir::Instruction>& inst) {
  if (auto opInst{ dynCastRef<OpInst>(inst) }) {
    visit(opInst);
  } else if (auto unaryInst{ dynCastRef<ir::UnaryInst>(inst) }) {
    visit(unaryInst);
  } else if (auto mvInst{ dynCastRef<MoveInst>(inst) }) {
    visit(mvInst);
  } else if (auto loadInst{ dynCastRef<LoadInst>(inst) }) {
    visit(loadInst);
  } else if (auto storeInst{ dynCastRef<StoreInst>(inst) }) {
    visit(storeInst);
  } else if (auto brInst{ dynCastRef<ir::BranchInst>(inst) }) {
    visit(brInst);
  } else if (auto callInst{ dynCastRef<ir::CallInst>(inst) }) {
    visit(callInst);
  } else if (auto retInst{ dynCastRef<ir::RetInst>(inst) }) {
    visit(retInst);
  } else if (auto allocaInst{ dynCastRef<ir::AllocaInst>(inst) }) {
    ///
  } else {
    bort_assert_nomsg(false);
  }
}

void InstructionVisitorBase::run(ir::Module& module) {
  for (auto&& func : module) {
    for (m_CurrentBBIter = func.begin(); m_CurrentBBIter != func.end();
         m_CurrentBBIter++) {
      auto& bb{ *m_CurrentBBIter };
      for (m_CurrentInstIter = bb.begin(); m_CurrentInstIter != bb.end();
           m_CurrentInstIter++) {
        genericVisit(*m_CurrentInstIter);
      }
    }
  }
}

} // namespace bort::codegen
