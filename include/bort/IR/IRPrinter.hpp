#pragma once
#include "bort/IR/AllocaInst.hpp"
#include "bort/IR/IRCodegen.hpp"
#include "bort/IR/Instruction.hpp"
#include "bort/IR/OpInst.hpp"

namespace bort::ir {

class IRPrinter {
public:
  void print(const Module& module);

private:
  void visit(const Ref<OpInst>& opInst);
  void visit(const Ref<AllocaInst>& allocaInst);
};

} // namespace bort::ir
