#include "bort/Codegen/RISCVCodegen.hpp"
#include "bort/Basic/Assert.hpp"
#include "bort/Basic/Casts.hpp"
#include "bort/CLI/IO.hpp"
#include "bort/Codegen/InstructionVisitorBase.hpp"
#include "bort/Codegen/LoadInst.hpp"
#include "bort/Codegen/MachineRegister.hpp"
#include "bort/Codegen/StoreInst.hpp"
#include "bort/Codegen/Utils.hpp"
#include "bort/Codegen/ValueLoc.hpp"
#include "bort/IR/AllocaInst.hpp"
#include "bort/IR/BasicBlock.hpp"
#include "bort/IR/BranchInst.hpp"
#include "bort/IR/CallInst.hpp"
#include "bort/IR/Constant.hpp"
#include "bort/IR/Instruction.hpp"
#include "bort/IR/Metadata.hpp"
#include "bort/IR/Module.hpp"
#include "bort/IR/MoveInst.hpp"
#include "bort/IR/OpInst.hpp"
#include "bort/IR/Register.hpp"
#include "bort/IR/Value.hpp"
#include "bort/IR/VariableUse.hpp"
#include "bort/Lex/Token.hpp"
#include <algorithm>
#include <cul/cul.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <memory>
#include <utility>
#include <vector>

using namespace bort::ir;

namespace bort::codegen::rv {

auto RVFuncAdditionalCode::toString() const -> std::string {
  return fmt::format("rv_code <...>");
}

auto RVInstInfo::toString() const -> std::string {
  return fmt::format("rv_ii .inst_name={}", InstName);
}

class PreprocessVisitor : public InstructionVisitorBase {
private:
  void visit(const Ref<ir::OpInst>& opInst) override {
    // eliminate immediatness
    for (int i{ 1 }; i <= 2; i++) {
      if (auto srcConst{ dynCastRef<Constant>(opInst->getOperand(i)) }) {
        auto newIRReg{ Register::getOrCreate(srcConst->getType()) };
        m_CurrentBBIter->insertBefore(
            m_CurrentInstIter, makeRef<MoveInst>(newIRReg, srcConst));
        opInst->setOperand(i, newIRReg);
      }

      if (opInst->getOp() == TokenKind::Plus) {
        // add can be immediate
        break;
      }
    }
  }
};

class InstructionChoiceVisitor : public InstructionVisitorBase {
private:
  void visit(const Ref<ir::OpInst>& opInst) override {
    static constexpr cul::BiMap s_OpInstNames{ [](auto&& selector) {
      return selector.Case(TokenKind::Plus, "add")
          .Case(TokenKind::Minus, "sub")
          .Case(TokenKind::Star, "mul")
          .Case(TokenKind::Div, "div")
          .Case(TokenKind::Less, "slt")
          .Case(TokenKind::Greater, "sgt");
    } };

    bort_assert(s_OpInstNames.Find(opInst->getOp()).has_value(),
                "Unknown op name");
    RVInstInfo info{ std::string{
        s_OpInstNames.Find(opInst->getOp()).value() } };
    if (isaRef<Constant>(opInst->getSrc2())) {
      info.InstName += "i";
    }
    opInst->addMDNode(info);
  }

  void visit(const Ref<ir::BranchInst>& brInst) override {
    RVInstInfo info{ "j" };
    if (brInst->isConditional()) {
      info.InstName = "bnez";
      if (brInst->isNegated()) {
        info.InstName = "beqz";
      }
    }

    brInst->addMDNode(std::move(info));
  }

  void visit(const Ref<ir::MoveInst>& mvInst) override {
    RVInstInfo info{ "li" };
    if (isaRef<MachineRegister>(mvInst->getSrc())) {
      info.InstName = "mv";
    }

    mvInst->addMDNode(std::move(info));
  }

  void visit(const Ref<LoadInst>& loadInst) override {
    RVInstInfo info{ "lw" };
    if (loadInst->getBytes() == 1) {
      info.InstName = "lb";
    }
    loadInst->addMDNode(std::move(info));
  }

  void visit(const Ref<StoreInst>& storeInst) override {
    RVInstInfo info{ "sw" };
    if (storeInst->getBytes() == 1) {
      info.InstName = "sb";
    }
    storeInst->addMDNode(std::move(info));
  }

  void visit(const Ref<CallInst>& callInst) override {
    RVInstInfo info{ "call" };
    callInst->addMDNode(std::move(info));
  }
};

struct FrameOffset : public Metadata {
  explicit FrameOffset(size_t offset)
      : Offset{ offset } {
  }

  [[nodiscard]] auto toString() const -> std::string override {
    return fmt::format("stack_loc .off={}", Offset);
  }

  size_t Offset;
};

auto RVMachineRegister::get(GPR gprId) -> Ref<RVMachineRegister> {
  static std::array<Ref<RVMachineRegister>, s_NumRegisters> s_Registry;
  static bool s_RegistryInitialized{ false };
  if (!s_RegistryInitialized) {
    for (int i{ 0 }; i < s_NumRegisters; i++) {
      GPR reg{ static_cast<GPR>(i) };
      s_Registry.at(i) = Ref<RVMachineRegister>{ new RVMachineRegister{
          reg, std::string{ GPRToString(reg) } } };
    }
    s_RegistryInitialized = true;
  }

  return s_Registry.at(static_cast<int>(gprId));
}

auto Generator::tryFindRegisterWithOperand(const Ref<ir::Operand>& op)
    -> std::optional<RVMachineRegisterRef> {
  bort_assert(m_OperandLocs.contains(op),
              "Operand not added to descriptors");

  auto& operandLocs{ m_OperandLocs.at(op) };

  auto existingRegisterLocIt{ std::ranges::find_if(
      operandLocs, [](const auto& loc) {
        return loc->getKind() == LocationKind::Register;
      }) };
  if (existingRegisterLocIt != operandLocs.end()) {
    auto registerLoc{ dynCastRef<RegisterLoc>(*existingRegisterLocIt) };
    return dynCastRef<RVMachineRegister>(registerLoc->getRegister());
  }
  return std::nullopt;
}

auto Generator::chooseReadReg(const Ref<ir::Operand>& op)
    -> RVMachineRegisterRef {
  auto existingRegister{ tryFindRegisterWithOperand(op) };
  if (existingRegister.has_value()) {
    return existingRegister.value();
  }

  auto freeRegIt{ std::ranges::find_if(m_RegisterContent, [](auto pair) {
    return pair.second.empty();
  }) };

  bort_assert(freeRegIt != m_RegisterContent.end(),
              "Spill not implemented");
  return freeRegIt->first;
}

auto Generator::chooseDstReg(const Ref<ir::Operand>& op)
    -> RVMachineRegisterRef {
  auto existingRegister{ tryFindRegisterWithOperand(op) };
  if (existingRegister.has_value()) {
    return existingRegister.value();
  }

  auto freeRegIt{ std::ranges::find_if(
      m_RegisterContent, [&op](auto pair) {
        auto& ops{ pair.second };
        return (ops.size() == 1 && ops.contains(op)) || ops.empty();
      }) };

  bort_assert(freeRegIt != m_RegisterContent.end(),
              "Spill not implemented");
  return freeRegIt->first;
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

auto GPRToString(GPR gpr) -> std::string_view {
  bort_assert(s_GPRStrings.Find(gpr).has_value(), "Unnamed register");
  return s_GPRStrings.Find(gpr).value();
}

void Generator::assignLocalOperandsOffsets() {
  for (auto&& func : m_Module) {
    /// 4 for ra + 4 for fp
    size_t offset{ 8 };
    auto operands{ getUniqueOperands(func) };
    for (auto&& operand : operands) {
      operand->addMDNode(FrameOffset{ offset });
      // @todo: pack variables and add alignment at the end
      offset += 4;
      // offset += operand->getType()->getSizeof();
    }

    auto alignment{ 16 - (offset % 16) };
    offset += alignment;
    FrameInfo frameInfo;
    frameInfo.Size         = offset;
    frameInfo.NumVariables = operands.size();
    func.addMDNode(frameInfo);
  }
}

void Generator::spillEndBB() {
  for (auto& [op, locs] : m_OperandLocs) {
    Ref<RegisterLoc> regLoc{ nullptr };
    Ref<ValueLoc> inMemoryLoc{ nullptr };
    for (auto&& loc : locs) {
      if (loc->getKind() == LocationKind::Register) {
        regLoc = dynCastRef<RegisterLoc>(loc);
      } else {
        inMemoryLoc = dynCastRef<ValueLoc>(loc);
      }
    }

    if (!inMemoryLoc) {
      bort_assert(regLoc, "Operand is neither in memory nor register");
      auto* frameOffset{ op->getMDNode<FrameOffset>() };
      generateStore(op, regLoc->getRegister(),
                    makeRef<StackLoc>(frameOffset->Offset));
    }
  }
}

static void attachAdditionalCode(IRFunction& func) {
  static size_t returnLabelCounter{ 0 };
  auto* frameInfo{ func.getMDNode<FrameInfo>() };
  RVFuncAdditionalCode functionPrologueEpilogue;
  functionPrologueEpilogue.Prologue =
      fmt::format("addi sp, sp, -{}\nsw ra, {}(sp)\nsw fp, "
                  "{}(sp)\naddi fp, sp, {}\n",
                  frameInfo->Size, frameInfo->Size - 4,
                  frameInfo->Size - 8, frameInfo->Size);
  std::string retCode{ "ret" };
  // probably not the best way
  if (func.getName() == "main") {
    retCode = "li a7, 10\necall\n";
  }
  functionPrologueEpilogue.Epilogue = fmt::format(
      ".L{}_ret:\nlw ra, {}(sp)\nlw fp, {}(sp)\naddi sp, sp, {}\n{}\n",
      returnLabelCounter++, frameInfo->Size - 4, frameInfo->Size - 8,
      frameInfo->Size, retCode);
  func.addMDNode(functionPrologueEpilogue);
}

void Generator::generate() {
  PreprocessVisitor().run(m_Module);
  assignLocalOperandsOffsets();
  for (auto&& func : m_Module) {
    attachAdditionalCode(func);
    for (m_CurrentBBIter = func.begin(); m_CurrentBBIter != func.end();
         m_CurrentBBIter++) {
      auto& bb{ *m_CurrentBBIter };
      reinitDescriptors(bb);
      for (m_CurrentInstIter = bb.begin(); m_CurrentInstIter != bb.end();
           m_CurrentInstIter++) {
        auto isLast{ ++InstIter{ m_CurrentInstIter } == bb.end() };
        auto isBranch{ isaRef<BranchInst>(*m_CurrentInstIter) };
        if (isLast && isBranch) {
          // spill before branch
          spillEndBB();
        }
        genericVisit(*m_CurrentInstIter);
        if (isLast && !isBranch) {
          // spill after other
          m_CurrentInstIter++;
          spillEndBB();
          break;
        }
      }
    }
  }
  InstructionChoiceVisitor().run(m_Module);
}

void Generator::generateLoad(const Ref<ir::Operand>& op,
                             const RVMachineRegisterRef& reg) {
  m_RegisterContent[reg].clear();
  m_RegisterContent[reg].insert(op);

  auto newLoc{ makeRef<RegisterLoc>(reg) };
  m_OperandLocs[op].insert(newLoc);

  auto memoryLocIt{ std::ranges::find_if_not(
      m_OperandLocs[op], [](const auto& loc) {
        return loc->getKind() == LocationKind::Register;
      }) };
  bort_assert(memoryLocIt != m_OperandLocs[op].end(),
              "Operand is neither in memory nor register");
  m_CurrentBBIter->insertBefore(
      m_CurrentInstIter,
      makeRef<LoadInst>(reg, *memoryLocIt, op->getType()->getSizeof()));
}

void Generator::generateStore(const Ref<ir::Operand>& op,
                              const Ref<MachineRegister>& reg,
                              const Ref<ValueLoc>& loc) {
  m_OperandLocs[op].insert(loc);

  m_CurrentBBIter->insertBefore(
      m_CurrentInstIter,
      makeRef<StoreInst>(reg, loc, op->getType()->getSizeof()));
}

void Generator::visit(const Ref<ir::OpInst>& opInst) {
  /// @todo destination register should be chosen differently
  for (int i{ 1 }; i <= 2; i++) {
    auto op{ dynCastRef<Operand>(opInst->getOperand(i)) };
    if (op) {
      auto reg{ chooseReadReg(op) };
      if (!m_RegisterContent[reg].contains(op)) {
        generateLoad(op, reg);
      }

      opInst->setOperand(i, reg);
    }
  }

  if (auto dstOp{ dynCastRef<Operand>(opInst->getDestination()) }) {
    auto dstReg{ chooseDstReg(dstOp) };

    // remove register location from other operands
    for (auto& [op, locs] : m_OperandLocs) {
      std::erase_if(locs, [&dstReg](const auto& loc) {
        if (auto regLoc{ dynCastRef<RegisterLoc>(loc) }) {
          return regLoc->getRegister() == dstReg;
        }
        return false;
      });
    }
    // register now only has destination operand
    m_RegisterContent[dstReg] = { dstOp };
    // destination operand is now only in register
    m_OperandLocs[dstOp] = { makeRef<RegisterLoc>(dstReg) };

    opInst->setDestination(std::move(dstReg));
  }
}

void Generator::visit(const Ref<ir::BranchInst>& brInst) {
  if (brInst->isConditional()) {
    if (auto op{ dynCastRef<Operand>(brInst->getCondition()) }) {
      auto reg{ chooseReadReg(op) };
      if (!m_RegisterContent[reg].contains(op)) {
        generateLoad(op, reg);
      }

      brInst->setCondition(reg);
    }
  }
}

void Generator::visit(const Ref<ir::MoveInst>& mvInst) {
  RVMachineRegisterRef srcReg{ nullptr };
  if (auto srcOp{ dynCastRef<Operand>(mvInst->getSrc()) }) {
    srcReg = chooseReadReg(srcOp);
    if (!m_RegisterContent[srcReg].contains(srcOp)) {
      generateLoad(srcOp, srcReg);
    }
    mvInst->setSrc(std::move(srcReg));
  }

  bort_assert(isaRef<Operand>(mvInst->getDestination()),
              "Move inst with immediate destination");
  auto dstOp{ dynCastRef<Operand>(mvInst->getDestination()) };
  auto dstReg{ srcReg ? srcReg : chooseDstReg(dstOp) };

  // dstOp is now in register
  m_RegisterContent[dstReg].insert(dstOp);
  // dstOp is now ONLY in register
  m_OperandLocs[dstOp] = { makeRef<RegisterLoc>(dstReg) };

  mvInst->setDestination(std::move(dstReg));
}

void Generator::visit(const Ref<ir::CallInst>& callInst) {
  static constexpr std::array s_ArgRegisters{ GPR::a0, GPR::a1, GPR::a2,
                                              GPR::a3, GPR::a4, GPR::a5,
                                              GPR::a6, GPR::a7 };
  int argRegN{ 0 };
  for (size_t i{ 0 }; i < callInst->getNumArgs(); i++) {
    auto arg{ callInst->getArg(i) };
    auto argOp{ dynCastRef<Operand>(arg) };

    if (argRegN == s_ArgRegisters.size()) {
      bort_assert(false,
                  "Too many arguments, memory args not implemented");
    }

    auto argReg{ RVMachineRegister::get(s_ArgRegisters.at(argRegN++)) };
    if (argOp && !m_RegisterContent[argReg].contains(argOp)) {
      generateLoad(argOp, argReg);
    } else if (!argOp) {
      // it is immediate
      m_CurrentBBIter->insertBefore(m_CurrentInstIter,
                                    makeRef<MoveInst>(argReg, arg));
    }
    callInst->setArg(i, std::move(argReg));
  }

  if (callInst->isVoid()) {
    return;
  }

  auto dstOp{ dynCastRef<Operand>(callInst->getDestination()) };
  auto dstReg{ RVMachineRegister::get(GPR::a0) };
  m_RegisterContent[dstReg] = { dstOp };
  m_OperandLocs[dstOp]      = { makeRef<RegisterLoc>(dstReg) };
  callInst->setDestination(std::move(dstReg));
}

void Generator::reinitDescriptors(const BasicBlock& bb) {
  for (int i{ 0 }; i < static_cast<int>(GPR::COUNT); i++) {
    m_RegisterContent[RVMachineRegister::get(static_cast<GPR>(i))]
        .clear();
  }
  m_OperandLocs.clear();
  for (auto&& operand : getUniqueOperands(bb)) {
    if (!operand) {
      continue;
    }

    auto* opLoc{ operand->getMDNode<FrameOffset>() };
    bort_assert(opLoc, "Operand has no FrameOffset");
    m_OperandLocs[operand].insert(makeRef<StackLoc>(opLoc->Offset));
  }
}

} // namespace bort::codegen::rv
