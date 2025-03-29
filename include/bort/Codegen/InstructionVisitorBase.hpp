#pragma once
#include "bort/Basic/Assert.hpp"
#include "bort/IR/BasicBlock.hpp"
#include "bort/IR/BranchInst.hpp"
#include "bort/IR/CallInst.hpp"
#include "bort/IR/Instruction.hpp"
#include "bort/IR/LoadInst.hpp"
#include "bort/IR/Metadata.hpp"
#include "bort/IR/Module.hpp"
#include "bort/IR/MoveInst.hpp"
#include "bort/IR/OpInst.hpp"
#include "bort/IR/RetInst.hpp"
#include "bort/IR/StoreInst.hpp"
#include "bort/IR/UnaryInst.hpp"

namespace bort::codegen {

class InstructionVisitorBase {
public:
  virtual ~InstructionVisitorBase() = default;

  virtual void run(ir::Module& module);

protected:
  void genericVisit(const Ref<ir::Instruction>& inst);

  auto getCurrentModule() const -> ir::Module*;

private:
  virtual void visit(const Ref<ir::OpInst>& opInst) {
    visitUnhandled(opInst);
  }
  virtual void visit(const Ref<ir::UnaryInst>& unaryInst) {
    visitUnhandled(unaryInst);
  }
  virtual void visit(const Ref<ir::MoveInst>& mvInst) {
    visitUnhandled(mvInst);
  }
  virtual void visit(const Ref<ir::LoadInst>& loadInst) {
    visitUnhandled(loadInst);
  }
  virtual void visit(const Ref<ir::StoreInst>& storeInst) {
    visitUnhandled(storeInst);
  }
  virtual void visit(const Ref<ir::BranchInst>& brInst) {
    visitUnhandled(brInst);
  }
  virtual void visit(const Ref<ir::CallInst>& callInst) {
    visitUnhandled(callInst);
  }
  virtual void visit(const Ref<ir::RetInst>& retInst) {
    visitUnhandled(retInst);
  }
  virtual void visitUnhandled(const Ref<ir::Instruction>& /* inst */) {
  }

protected:
  ir::InstIter m_CurrentInstIter;
  ir::BBIter m_CurrentBBIter;
  ir::IRFuncIter m_CurrentFuncIter;

private:
  ir::Module* m_CurrentModule{nullptr};
};

struct RemoveInstructionMDTag : ir::MDTag {
  RemoveInstructionMDTag()
      : MDTag{ "rm" } {
  }
};

class InstructionRemover : public InstructionVisitorBase {
public:
  void run(ir::Module& module) override;
};

} // namespace bort::codegen
