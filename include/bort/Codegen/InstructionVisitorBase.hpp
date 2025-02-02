#pragma once
#include "bort/Codegen/LoadInst.hpp"
#include "bort/Codegen/StoreInst.hpp"
#include "bort/IR/BasicBlock.hpp"
#include "bort/IR/BranchInst.hpp"
#include "bort/IR/CallInst.hpp"
#include "bort/IR/Instruction.hpp"
#include "bort/IR/Module.hpp"
#include "bort/IR/MoveInst.hpp"
#include "bort/IR/OpInst.hpp"

namespace bort::codegen {

class InstructionVisitorBase {
public:
  virtual ~InstructionVisitorBase() = default;

  virtual void run(ir::Module& module);

protected:
  void genericVisit(const Ref<ir::Instruction>& inst);

private:
  virtual void visit(const Ref<ir::OpInst>& /* opInst */) {
  }
  virtual void visit(const Ref<ir::MoveInst>& /* mvInst */) {
  }
  virtual void visit(const Ref<LoadInst>& /* loadInst */) {
  }
  virtual void visit(const Ref<StoreInst>& /* storeInst */) {
  }
  virtual void visit(const Ref<ir::BranchInst>& /* brInst */) {
  }
  virtual void visit(const Ref<ir::CallInst>& /* callInst */) {
  }

protected:
  ir::InstIter m_CurrentInstIter;
  ir::BBIter m_CurrentBBIter;
};

} // namespace bort::codegen
