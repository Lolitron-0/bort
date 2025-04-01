#pragma once
#include "bort/Codegen/InstructionVisitorBase.hpp"
#include "bort/Codegen/RARSMacroCallInst.hpp"
#include "bort/IR/BranchInst.hpp"
#include "bort/IR/CallInst.hpp"
#include "bort/IR/Instruction.hpp"
#include "bort/IR/LoadInst.hpp"
#include "bort/IR/Module.hpp"
#include "bort/IR/MoveInst.hpp"
#include "bort/IR/OpInst.hpp"
#include "bort/IR/StoreInst.hpp"
#include "bort/IR/UnaryInst.hpp"
#include <ostream>

namespace bort::codegen::rv {

class Printer : public InstructionVisitorBase {
public:
  explicit Printer(std::ostream& stream)
      : m_Stream{ stream } {
  }

  void run(ir::Module& module) override;

private:
  void visit(const Ref<ir::OpInst>& opInst) override;
  void visit(const Ref<ir::UnaryInst>& unaryInst) override;
  void visit(const Ref<RARSMacroCallInst>& macroInst) override;
  void visit(const Ref<ir::BranchInst>& brInst) override;
  void visit(const Ref<ir::MoveInst>& mvInst) override;
  void visit(const Ref<ir::LoadInst>& loadInst) override;
  void visit(const Ref<ir::StoreInst>& storeInst) override;
  void visit(const Ref<ir::CallInst>& callInst) override;

  void printHeader(const ir::Module& module);

private:
  std::ostream& m_Stream;
};

} // namespace bort::codegen::rv
