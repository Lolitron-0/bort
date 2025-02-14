#pragma once
#include "bort/IR/AllocaInst.hpp"
#include "bort/IR/BranchInst.hpp"
#include "bort/IR/CallInst.hpp"
#include "bort/IR/Module.hpp"
#include "bort/IR/OpInst.hpp"
#include "bort/IR/RetInst.hpp"

namespace bort::ir {

class IRPrinter {
public:
  void print(const Module& module);

private:
  void visit(const Ref<OpInst>& opInst);
  void visit(const Ref<AllocaInst>& allocaInst);
  void visit(const Ref<BranchInst>& branchInst);
  void visit(const Ref<CallInst>& callInst);
  void visit(const Ref<RetInst>& retInst);
};

} // namespace bort::ir
