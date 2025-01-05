#include "bort/Codegen/RISCVCodegen.hpp"
#include "bort/Basic/Assert.hpp"
#include "bort/Basic/Casts.hpp"
#include "bort/Codegen/Utils.hpp"
#include "bort/IR/AllocaInst.hpp"
#include "bort/IR/BasicBlock.hpp"
#include "bort/IR/Instruction.hpp"
#include "bort/IR/LoadInst.hpp"
#include "bort/IR/OpInst.hpp"
#include "bort/IR/Value.hpp"
#include "bort/IR/ValueLoc.hpp"
#include "bort/IR/VariableUse.hpp"
#include "bort/Lex/Token.hpp"
#include <alloca.h>
#include <cul/cul.hpp>
#include <memory>

using namespace bort::ir;

namespace bort::codegen::rv {

RVMachineRegisterRef Generator::chooseUseReg(
    Ref<ir::Operand> op, InstIter it,
    std::initializer_list<GPR> ignoreRegisters) {
  return RVMachineRegister::get(GPR::t0);
}

static std::array<RVMachineRegisterRef, 3> getReg(InstIter it) {
  return { RVMachineRegister::get(GPR::t0),
           RVMachineRegister::get(GPR::t1),
           RVMachineRegister::get(GPR::t2) };
}

static constexpr cul::BiMap s_GPRStrings{ [](auto selector) {
  return selector.Case(GPR::t0, "t0")
      .Case(GPR::t1, "t1")
      .Case(GPR::t2, "t2")
      .Case(GPR::t3, "t3")
      .Case(GPR::t4, "t4")
      .Case(GPR::t5, "t5")
      .Case(GPR::t6, "t6")
      .Case(GPR::a0, "a0")
      .Case(GPR::a1, "a1")
      .Case(GPR::a2, "a2")
      .Case(GPR::a3, "a3")
      .Case(GPR::a4, "a4")
      .Case(GPR::a5, "a5")
      .Case(GPR::a6, "a6")
      .Case(GPR::a7, "a7")
      .Case(GPR::s0, "s0")
      .Case(GPR::s1, "s1")
      .Case(GPR::s2, "s2")
      .Case(GPR::s3, "s3")
      .Case(GPR::s4, "s4")
      .Case(GPR::s5, "s5")
      .Case(GPR::s6, "s6")
      .Case(GPR::s7, "s7")
      .Case(GPR::s8, "s8")
      .Case(GPR::s9, "s9")
      .Case(GPR::s10, "s10")
      .Case(GPR::s11, "s11");
} };

static auto getOperands(const Ref<OpInst>& opInst)
    -> std::array<Ref<Operand>, 3> {
  std::array<Ref<Operand>, 3> operands;

  operands[0] = dynCastRef<Operand>(opInst->getSrc1());
  bort_assert(operands[0], "Instruction destination is not Operand");

  if (opInst->getSrc2()) {
    operands[1] = dynCastRef<Operand>(opInst->getSrc2());
    bort_assert(operands[1], "Instruction destination is not Operand");
  }

  operands[2] = dynCastRef<Operand>(opInst->getDestination());
  bort_assert(operands[2], "Instruction destination is not Operand");

  return operands;
}

auto GPRToString(GPR gpr) -> std::string_view {
  bort_assert(s_GPRStrings.Find(gpr).has_value(), "GPR has no name");
  return s_GPRStrings.Find(gpr).value();
}

void Generator::assignLocalVariablesOffsets() {
  for (auto&& func : m_Module) {
    size_t offset{ 0 };
    auto operands{ getUniqueOperands(func) };
    for (auto&& operand : operands) {
      operand->addMDNode(StackLoc{ offset });
      offset += operand->getType()->getSizeof();
    }

    auto alignment{ 16 - (offset % 16) };
    offset += alignment;
    FrameInfo frameInfo;
    frameInfo.Size         = offset;
    frameInfo.NumVariables = operands.size();
    func.addMDNode(frameInfo);
  }
}

void Generator::generate() {
  assignLocalVariablesOffsets();
  // for (auto&& func : m_Module) {
  //   for (auto&& bb : func) {
  //     for (auto it{ bb.begin() }; it != bb.end(); it++) {
  //       processInst(it, bb);
  //     }
  //   }
  // }
}

void Generator::processInst(InstIter it, ir::BasicBlock& bb) {
  auto& inst{ *it };
  if (auto opInst{ std::dynamic_pointer_cast<ir::OpInst>(inst) }) {
    auto [op1, op2, dst]      = getOperands(opInst);
    auto [Rsrc1, Rsrc2, Rdst] = getReg(it);
  }
}

void Generator::reinitDescriptors(const BasicBlock& bb) {
  for (auto&& inst : bb) {
    /// @todo find operands used in this instruction, they should
    /// already have some info about location (ir registers?)
  }
}

} // namespace bort::codegen::rv
