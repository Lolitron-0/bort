#pragma once
#include "bort/IR/AllocaInst.hpp"
#include "bort/IR/BranchInst.hpp"
#include "bort/IR/CallInst.hpp"
#include "bort/IR/LoadInst.hpp"
#include "bort/IR/Module.hpp"
#include "bort/IR/OpInst.hpp"
#include "bort/IR/RetInst.hpp"
#include "bort/IR/StoreInst.hpp"
#include "bort/IR/UnaryInst.hpp"

namespace bort::ir {

class IRPrinter {
public:
  void print(const Module& module);

private:
  void visit(const Ref<OpInst>& opInst);
  void visit(const Ref<UnaryInst>& unaryInst);
  void visit(const Ref<AllocaInst>& allocaInst);
  void visit(const Ref<BranchInst>& branchInst);
  void visit(const Ref<CallInst>& callInst);
  void visit(const Ref<RetInst>& retInst);
  void visit(const Ref<LoadInst>& loadInst);
  void visit(const Ref<StoreInst>& storeInst);
};

} // namespace bort::ir
