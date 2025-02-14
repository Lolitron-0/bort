#pragma once
#include "bort/Codegen/InstructionVisitorBase.hpp"
#include "bort/Codegen/LoadInst.hpp"
#include "bort/Codegen/StoreInst.hpp"
#include "bort/IR/BranchInst.hpp"
#include "bort/IR/CallInst.hpp"
#include "bort/IR/Module.hpp"
#include "bort/IR/MoveInst.hpp"
#include "bort/IR/OpInst.hpp"
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
  void visit(const Ref<ir::BranchInst>& brInst) override;
  void visit(const Ref<ir::MoveInst>& mvInst) override;
  void visit(const Ref<LoadInst>& loadInst) override;
  void visit(const Ref<StoreInst>& storeInst) override;
  void visit(const Ref<ir::CallInst>& callInst) override;

  void printHeader();

private:
  std::ostream& m_Stream;
};

} // namespace bort::codegen::rv
