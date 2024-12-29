#pragma once
#include "bort/IR/AllocaInst.hpp"
#include "bort/IR/Instruction.hpp"
#include "bort/IR/OpInst.hpp"

namespace bort::ir {

class IRPrinter {
public:
  void print(const std::vector<Ref<Instruction>>& instructions);

private:
  void visit(const Ref<OpInst>& opInst);
  void visit(const Ref<AllocaInst>& allocaInst);
};

} // namespace bort::ir
