#pragma once
#include "bort/IR/AllocaInst.hpp"
#include "bort/IR/BranchInst.hpp"
#include "bort/IR/Instruction.hpp"
#include "bort/IR/Module.hpp"
#include "bort/IR/OpInst.hpp"

namespace bort::ir {

class IRPrinter {
public:
  void print(const Module& module);

private:
  void visit(const Ref<OpInst>& opInst);
  void visit(const Ref<AllocaInst>& allocaInst);
  void visit(const Ref<BranchInst>& branchInst);
};

} // namespace bort::ir
