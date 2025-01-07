#include "bort/Basic/Assert.hpp"
#include "bort/Codegen/InstructionVisitorBase.hpp"
#include "bort/Codegen/StoreInst.hpp"
#include "bort/IR/AllocaInst.hpp"

using namespace bort::ir;

namespace bort::codegen {

void InstructionVisitorBase::genericVisit(
    const Ref<ir::Instruction>& inst) {
  if (auto opInst{ dynCastRef<OpInst>(inst) }) {
    visit(opInst);
  } else if (auto mvInst{ dynCastRef<MoveInst>(inst) }) {
    visit(mvInst);
  } else if (auto loadInst{ dynCastRef<LoadInst>(inst) }) {
    visit(loadInst);
  } else if (auto storeInst{ dynCastRef<StoreInst>(inst) }) {
    visit(storeInst);
  } else if (auto brInst{ dynCastRef<ir::BranchInst>(inst) }) {
    visit(brInst);
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
