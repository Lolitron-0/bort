#include "bort/Codegen/RISCVCodegen.hpp"
#include "bort/Basic/Assert.hpp"
#include "bort/Basic/Casts.hpp"
#include "bort/Codegen/InstructionVisitorBase.hpp"
#include "bort/Codegen/Intrinsics.hpp"
#include "bort/Codegen/MachineRegister.hpp"
#include "bort/Codegen/RARSMacroCallInst.hpp"
#include "bort/Codegen/Utils.hpp"
#include "bort/Codegen/ValueLoc.hpp"
#include "bort/Frontend/Type.hpp"
#include "bort/IR/BasicBlock.hpp"
#include "bort/IR/BranchInst.hpp"
#include "bort/IR/CallInst.hpp"
#include "bort/IR/Constant.hpp"
#include "bort/IR/GepInst.hpp"
#include "bort/IR/LoadInst.hpp"
#include "bort/IR/Metadata.hpp"
#include "bort/IR/Module.hpp"
#include "bort/IR/MoveInst.hpp"
#include "bort/IR/OpInst.hpp"
#include "bort/IR/Register.hpp"
#include "bort/IR/RetInst.hpp"
#include "bort/IR/StoreInst.hpp"
#include "bort/IR/UnaryInst.hpp"
#include "bort/IR/Value.hpp"
#include "bort/IR/VariableUse.hpp"
#include "bort/Lex/Token.hpp"
#include "cul/BiMap.hpp"
#include <algorithm>
#include <boost/range/adaptor/indexed.hpp>
#include <cul/cul.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace bort::ir;

namespace bort::codegen::rv {

auto RVFuncPrologueEpilogue::toString() const -> std::string {
  return fmt::format("rv_code <...>");
}

auto RVInstInfo::toString() const -> std::string {
  return fmt::format("rv_ii .inst_name={}", InstName);
}

auto RARSMacroDefinitions::toString() const -> std::string {
  return fmt::format("rars_macros .num_defined={}", Macros.size());
}

auto RVBranchInfo::toString() const -> std::string {
  return fmt::format("rv_bri .is_rhs_zero={}", NoRHS);
}

struct MemoryDependencyMDTag : MDTag {
  MemoryDependencyMDTag()
      : MDTag{ "mem_dependency" } {
  }
};

static void markForRemoval(const Ref<ir::Instruction>& inst) {
  inst->addMDNode(RemoveInstructionMDTag{});
}

/// Operates on IR only. Removes immediate operands from certain
/// instructions, desugars some constructions
class PreprocessPass : public InstructionVisitorBase {
private:
  void visit(const Ref<ir::OpInst>& opInst) override {
    // eliminate immediatness
    for (int i{ OpInst::Src1Idx }; i <= OpInst::Src2Idx; i++) {
      if (auto srcConst{ dynCastRef<Constant>(opInst->getOperand(i)) }) {
        opInst->setOperand(i, moveToNewReg(srcConst));
      }

      if (opInst->getOp() == TokenKind::Plus ||
          opInst->getOp() == TokenKind::Amp ||
          opInst->getOp() == TokenKind::Pipe) {
        // add can be immediate
        break;
      }
    }
  }

  void visit(const Ref<ir::UnaryInst>& unaryInst) override {
    if (unaryInst->getOp() == TokenKind::Amp) {
      unaryInst->getSrc()->addMDNode(MemoryDependencyMDTag{});
    }

    if (auto srcConst{ dynCastRef<Constant>(unaryInst->getSrc()) }) {
      unaryInst->setSrc(moveToNewReg(srcConst));
    }
  }

  void visit(const Ref<GepInst>& gepInst) override {
    if (auto indexConstant{ dynCastRef<Constant>(gepInst->getIndex()) }) {
      gepInst->setIndex(moveToNewReg(indexConstant));
    }
  }

  /// %arrPtr = addr %arr
  /// %valueReg = GA[0]
  /// store %valueReg, %arrPtr
  /// %arrPtr = add %arrPtr, %elementSize
  /// ...
  void processGlobalArrayAssignment(const Ref<ir::MoveInst>& inst,
                                    const Ref<ir::GlobalArray>& GA) {
    auto arrTy{ dynCastRef<ArrayType>(GA->getType()) };
    auto arrPtrTy{ PointerType::get(arrTy->getBaseType()) };
    auto arrPtr{ Register::getOrCreate(arrPtrTy) };
    auto elementSize{ IntegralConstant::getInt(
        arrTy->getBaseType()->getSizeof()) };
    auto valueReg{ Register::getOrCreate(arrPtrTy->getPointee()) };

    auto lastIter{ m_CurrentInstIter };
    auto addInstruction{ [&](auto&& inst) {
      m_CurrentBBIter->insertAfter(lastIter, inst);
      lastIter++;
    } };

    addInstruction(makeRef<UnaryInst>(TokenKind::Amp, arrPtr,
                                      inst->getDestination()));
    for (auto&& el : GA->getValues() | boost::adaptors::indexed()) {
      addInstruction(makeRef<MoveInst>(valueReg, el.value()));
      addInstruction(makeRef<StoreInst>(valueReg, arrPtr, elementSize));
      if (static_cast<size_t>(el.index()) != GA->getValues().size() - 1) {
        addInstruction(makeRef<OpInst>(TokenKind::Plus, arrPtr, arrPtr,
                                       elementSize));
      }
    }

    markForRemoval(inst);
  }

  void visit(const Ref<MoveInst>& moveInst) override {
    if (auto GA{ dynCastRef<GlobalArray>(moveInst->getSrc()) }) {
      processGlobalArrayAssignment(moveInst, GA);
      return;
    }
  }

  void visit(const Ref<ir::BranchInst>& brInst) override {
    if (!brInst->isConditional()) {
      return;
    }

    RVBranchInfo info{ false };
    auto [lhs, rhs]{ brInst->getOperands() };
    if (auto lhsConst{ dynCastRef<Constant>(lhs) }) {
      lhs = moveToNewReg(lhsConst);
    }

    auto rhsConstant{ dynCastRef<Constant>(rhs) };
    auto rhsIntConstant{ dynCastRef<IntegralConstant>(rhs) };
    if (rhsConstant) {
      if (rhsIntConstant && rhsIntConstant->getValue() == 0) {
        info.NoRHS = true;
        rhs        = nullptr;
      } else {
        rhs = moveToNewReg(rhsConstant);
      }
    }

    brInst->setOperands(lhs, rhs);
    brInst->addMDNode(info);
  }

  void visit(const Ref<StoreInst>& storeInst) override {
    if (auto srcConstant{
            dynCastRef<Constant>(storeInst->getSource()) }) {
      storeInst->setSource(moveToNewReg(srcConstant));
    }
  }

  auto moveToNewReg(const Ref<Constant>& constant) -> Ref<Register> {
    auto newIRReg{ Register::getOrCreate(constant->getType()) };
    m_CurrentBBIter->insertBefore(m_CurrentInstIter,
                                  makeRef<MoveInst>(newIRReg, constant));
    return newIRReg;
  }
};

class InstructionChoicePass : public InstructionVisitorBase {
private:
  void visit(const Ref<ir::OpInst>& opInst) override {
    static constexpr cul::BiMap s_OpInstNames{ [](auto&& selector) {
      return selector.Case(TokenKind::Plus, "add")
          .Case(TokenKind::Minus, "sub")
          .Case(TokenKind::Star, "mul")
          .Case(TokenKind::Div, "div")
          .Case(TokenKind::Less, "slt")
          .Case(TokenKind::Greater, "sgt")
          .Case(TokenKind::Amp, "and")
          .Case(TokenKind::Pipe, "or");
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

  void visit(const Ref<UnaryInst>& unaryInst) override {
    static constexpr cul::BiMap s_UnaryInstNames{ [](auto&& selector) {
      return selector.Case(TokenKind::Minus, "neg");
    } };

    bort_assert(s_UnaryInstNames.Find(unaryInst->getOp()).has_value(),
                "Unknown unary name");

    RVInstInfo info{ std::string{
        s_UnaryInstNames.Find(unaryInst->getOp()).value() } };
    unaryInst->addMDNode(info);
  }

  void visit(const Ref<ir::BranchInst>& brInst) override {
    static constexpr cul::BiMap s_BranchModePostfix{ [](auto&& selector) {
      return selector.Case(TokenKind::Equals, "eq")
          .Case(TokenKind::NotEquals, "ne")
          .Case(TokenKind::Greater, "gt")
          .Case(TokenKind::GreaterEqual, "ge")
          .Case(TokenKind::Less, "lt")
          .Case(TokenKind::LessEqual, "le");
    } };

    RVInstInfo info{ "j" };
    if (brInst->isConditional()) {
      bort_assert(s_BranchModePostfix.Find(brInst->getMode()).has_value(),
                  "Branch mode not resolved in codegen");
      info.InstName = fmt::format(
          "b{}", s_BranchModePostfix.Find(brInst->getMode()).value());

      auto* brInfo = brInst->getMDNode<RVBranchInfo>();
      if (brInfo->NoRHS) {
        info.InstName += "z";
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
    RVInstInfo info{ "l?" };
    switch (loadInst->getBytes()->getValue()) {
    case 1:
      info.InstName = "lb";
      break;
    case 4:
      info.InstName = "lw";
      break;
    }
    loadInst->addMDNode(std::move(info));
  }

  void visit(const Ref<StoreInst>& storeInst) override {
    RVInstInfo info{ "s?" };
    switch (storeInst->getBytes()->getValue()) {
    case 1:
      info.InstName = "sb";
      break;
    case 4:
      info.InstName = "sw";
      break;
    }
    storeInst->addMDNode(std::move(info));
  }

  void visit(const Ref<CallInst>& callInst) override {
    RVInstInfo info{ "call" };
    callInst->addMDNode(std::move(info));
  }
};

class StoreSourceOperandMD : public Metadata {
public:
  explicit StoreSourceOperandMD(Ref<ir::Operand> op)
      : Operand{ std::move(op) } {
  }

  [[nodiscard]] auto toString() const -> std::string override {
    return fmt::format("store_op .op=%{}", Operand->getName());
  }

  Ref<ir::Operand> Operand;
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
  static std::unordered_map<GPR, Ref<RVMachineRegister>> s_Registry;
  static bool s_RegistryInitialized{ false };
  if (!s_RegistryInitialized) {
    for (int i{ static_cast<int>(GPR::VALUE_REGS_START) };
         i != static_cast<int>(GPR::VALUE_REGS_END); i++) {
      GPR reg{ static_cast<GPR>(i) };
      s_Registry[reg] = Ref<RVMachineRegister>{ new RVMachineRegister{
          reg, std::string{ GPRToString(reg) } } };
    }

    for (int i{ static_cast<int>(GPR::SPECIAL_REGS_START) };
         i != static_cast<int>(GPR::SPECIAL_REGS_END); i++) {
      GPR reg{ static_cast<GPR>(i) };
      s_Registry[reg] = Ref<RVMachineRegister>{ new RVMachineRegister{
          reg, std::string{ GPRToString(reg) } } };
    }
    s_RegistryInitialized = true;
  }

  return s_Registry.at(gprId);
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
      .Case(GPR::s11, "s11")
      .Case(GPR::sp, "sp")
      .Case(GPR::ra, "ra")
      .Case(GPR::fp, "fp");
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

void Generator::fillOperandUsages() {
  for (auto&& func : m_Module) {
    for (auto&& bb : func) {
      auto operands{ getUniqueOperands(bb) };
      for (auto&& operand : operands) {
        m_OperandUsages[operand].insert(&bb);
      }
    }
  }
}

auto Generator::getOperandRegisterMemoryLocs(const Ref<ir::Operand>& op)
    const -> std::pair<Ref<RegisterLoc>, Ref<ValueLoc>> {
  Ref<RegisterLoc> regLoc{ nullptr };
  Ref<ValueLoc> inMemoryLoc{ nullptr };
  for (auto&& loc : m_OperandLocs.at(op)) {
    if (loc->getKind() == LocationKind::Register) {
      regLoc = dynCastRef<RegisterLoc>(loc);
    } else {
      inMemoryLoc = dynCastRef<ValueLoc>(loc);
    }
  }
  return { regLoc, inMemoryLoc };
}

auto Generator::notLocalToBBFilter(const Ref<ir::Operand>& op,
                                   const ir::BasicBlock& bb) -> bool {
  return isaRef<VariableUse>(op) ||
         std::count_if(m_OperandUsages[op].begin(),
                       m_OperandUsages[op].end(),
                       [&bb](const auto* usageBB) {
                         return usageBB != &bb;
                       }) > 0;
}

void Generator::spillIf(const SpillFilter& filter) {
  for (auto& [op, _] : m_OperandLocs) {
    if (!filter(op)) {
      continue;
    }

    auto [regLoc, inMemoryLoc]{ getOperandRegisterMemoryLocs(op) };

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
  /// TODO: move to ret inst
  RVFuncPrologueEpilogue functionPrologueEpilogue;
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
  auto retLabel = fmt::format(".L{}_ret", returnLabelCounter++);
  auto* retBB{ func.addBB(retLabel) };
  functionPrologueEpilogue.EpilogueBB = retBB;
  retBB->addInstruction(makeRef<LoadInst>(
      RVMachineRegister::get(GPR::ra),
      makeRef<StackLoc>(frameInfo->Size - 4),
      IntegralConstant::getInt(IntType::get()->getSizeof())));
  retBB->addInstruction(makeRef<LoadInst>(
      RVMachineRegister::get(GPR::fp),
      makeRef<StackLoc>(frameInfo->Size - 8),
      IntegralConstant::getInt(IntType::get()->getSizeof())));
  retBB->addInstruction(
      makeRef<OpInst>(TokenKind::Plus, RVMachineRegister::get(GPR::sp),
                      RVMachineRegister::get(GPR::sp),
                      IntegralConstant::getInt(frameInfo->Size)));
  functionPrologueEpilogue.Epilogue = fmt::format("{}\n\n", retCode);
  func.addMDNode(functionPrologueEpilogue);
}

static constexpr std::array ArgRegisters{ GPR::a0, GPR::a1, GPR::a2,
                                          GPR::a3, GPR::a4, GPR::a5,
                                          GPR::a6, GPR::a7 };

void Generator::generate() {
  m_Module.addMDNode(RARSMacroDefinitions{});
  PreprocessPass().run(m_Module);
  InstructionRemover().run(m_Module);

  fillOperandUsages();
  assignLocalOperandsOffsets();
  for (m_CurrentFuncIter = m_Module.begin();
       m_CurrentFuncIter != m_Module.end(); m_CurrentFuncIter++) {
    auto& func{ *m_CurrentFuncIter };
    attachAdditionalCode(func);
    for (m_CurrentBBIter = func.begin(); m_CurrentBBIter != func.end();
         m_CurrentBBIter++) {
      auto& bb{ *m_CurrentBBIter };

      reinitDescriptors(bb, m_CurrentBBIter == func.begin());

      SpillFilter nonLocalSpillFilter{ [this, &bb](const auto& op) {
        return notLocalToBBFilter(op, bb);
      } };

      for (m_CurrentInstIter = bb.begin(); m_CurrentInstIter != bb.end();
           m_CurrentInstIter++) {
        auto isLast{ ++InstIter{ m_CurrentInstIter } == bb.end() };
        auto isJump{ isJumpInst(*m_CurrentInstIter) };
        if (isLast && isJump) {
          // spill before branch
          spillIf(nonLocalSpillFilter);
        }
        genericVisit(*m_CurrentInstIter);
        if (isLast && !isJump) {
          // spill after other
          m_CurrentInstIter++;
          spillIf(nonLocalSpillFilter);
          break;
        }
      }
    }
  }

  InstructionRemover().run(m_Module);
  InstructionChoicePass().run(m_Module);

  auto* macros{ m_Module.getMDNode<RARSMacroDefinitions>() };
  for (auto&& macroID : m_UsedMacros) {
    macros->Macros.push_back(intrinsics::getDefinition(macroID));
  }
}

void Generator::generateLoad(const Ref<ir::Operand>& op,
                             const RVMachineRegisterRef& reg) {
  m_RegisterContent[reg].clear();
  m_RegisterContent[reg].insert(op);

  auto oldRegLocIt{ std::ranges::find_if(
      m_OperandLocs[op], [](const auto& loc) {
        return loc->getKind() == LocationKind::Register;
      }) };

  auto newLoc{ makeRef<RegisterLoc>(reg) };
  m_OperandLocs[op].insert(newLoc);

  // if we found op in other register just move it to new one
  if (oldRegLocIt != m_OperandLocs[op].end()) {
    auto oldRegLoc{ dynCastRef<RegisterLoc>(*oldRegLocIt) };
    bort_assert(oldRegLoc,
                "Location has type Register and is not RegisterLoc");
    m_CurrentBBIter->insertBefore(
        m_CurrentInstIter,
        makeRef<MoveInst>(reg, oldRegLoc->getRegister()));
    return;
  }

  // otherwise load from memory
  auto memoryLocIt{ std::ranges::find_if_not(
      m_OperandLocs[op], [](const auto& loc) {
        return loc->getKind() == LocationKind::Register;
      }) };
  bort_assert(memoryLocIt != m_OperandLocs[op].end(),
              "Operand is neither in memory nor register");
  m_CurrentBBIter->insertBefore(
      m_CurrentInstIter,
      makeRef<LoadInst>(
          reg, *memoryLocIt,
          IntegralConstant::getInt(op->getType()->getSizeof())));
}

void Generator::generateStore(const Ref<ir::Operand>& op,
                              const Ref<MachineRegister>& reg,
                              const Ref<ValueLoc>& loc) {
  m_OperandLocs[op].insert(loc);

  auto storeInst{ makeRef<StoreInst>(
      reg, loc, IntegralConstant::getInt(op->getType()->getSizeof())) };
  storeInst->addMDNode(StoreSourceOperandMD{ op });
  m_CurrentBBIter->insertBefore(m_CurrentInstIter, std::move(storeInst));
}

void Generator::processSourceChoice(const RVMachineRegisterRef& reg,
                                    const Ref<ir::Operand>& op) {
  if (!m_RegisterContent[reg].contains(op)) {
    generateLoad(op, reg);
  }
}

void Generator::processDstChoice(const RVMachineRegisterRef& reg,
                                 const Ref<ir::Operand>& op) {
  // remove register location from other operands
  for (auto& [op, locs] : m_OperandLocs) {
    std::erase_if(locs, [&reg](const auto& loc) {
      if (auto regLoc{ dynCastRef<RegisterLoc>(loc) }) {
        return regLoc->getRegister() == reg;
      }
      return false;
    });
  }
  // register now only has destination operand
  m_RegisterContent[reg] = { op };
  // destination operand is now only in register
  m_OperandLocs[op] = { makeRef<RegisterLoc>(reg) };

  spillIf([](const auto& op) {
    return op->template getMDNode<MemoryDependencyMDTag>();
  });
}

void Generator::visit(const Ref<ir::OpInst>& opInst) {
  /// @todo destination register should be chosen differently
  for (int i{ 1 }; i <= 2; i++) {
    auto op{ dynCastRef<Operand>(opInst->getOperand(i)) };
    if (op) {
      auto reg{ chooseReadReg(op) };
      processSourceChoice(reg, op);

      opInst->setOperand(i, reg);
    }
  }

  if (auto dstOp{ dynCastRef<Operand>(opInst->getDestination()) }) {
    auto dstReg{ chooseDstReg(dstOp) };
    processDstChoice(dstReg, dstOp);

    opInst->setDestination(std::move(dstReg));
  }
}

void Generator::visit(const Ref<ir::UnaryInst>& unaryInst) {
  RVMachineRegisterRef dstReg;
  if (auto dstOp{ dynCastRef<Operand>(unaryInst->getDestination()) }) {
    dstReg = chooseDstReg(dstOp);
    processDstChoice(dstReg, dstOp);
    unaryInst->setDestination(dstReg);
  }

  if (unaryInst->getOp() == TokenKind::Amp) {
    bort_assert(unaryInst->getSrc()->getMDNode<MemoryDependencyMDTag>(),
                "Addr operand not marked as memory dependency");
    auto op{ dynCastRef<Operand>(unaryInst->getSrc()) };
    bort_assert(op, "Addr operand should be operand");
    auto [_, inMemoryLoc]{ getOperandRegisterMemoryLocs(op) };
    bort_assert(inMemoryLoc, "Addr operand memory consistency broken");
    bort_assert(dstReg, "Addr destination register not chosen");
    evaluateLocAddress(inMemoryLoc, dstReg);
    markForRemoval(unaryInst);
    return;
  }

  if (auto op{ dynCastRef<Operand>(unaryInst->getSrc()) }) {
    auto reg{ chooseReadReg(op) };
    processSourceChoice(reg, op);
    unaryInst->setSrc(std::move(reg));
  }
}

void Generator::visit(const Ref<ir::GepInst>& gepInst) {
  auto bpOp{ dynCastRef<Operand>(gepInst->getBasePtr()) };
  bort_assert(bpOp, "Gep base pointer should be operand");
  auto bpReg{ chooseReadReg(bpOp) };
  processSourceChoice(bpReg, bpOp);
  gepInst->setArray(bpReg);

  if (auto op{ dynCastRef<Operand>(gepInst->getIndex()) }) {
    auto reg{ chooseReadReg(op) };
    processSourceChoice(reg, op);
    gepInst->setIndex(reg);
  }

  gepInst->setDestination(gepInst->getBasePtr());
  processDstChoice(bpReg, bpOp);
  // elem.ptr %ptr %index %stride_shift
  m_CurrentBBIter->insertBefore(
      m_CurrentInstIter,
      createMacroCall(intrinsics::MacroID::ElemPtr, gepInst->getBasePtr(),
                      gepInst->getIndex(),
                      IntegralConstant::getInt(
                          std::log2(gepInst->getType()->getSizeof()))));
  markForRemoval(gepInst);
}

void Generator::visit(const Ref<ir::BranchInst>& brInst) {
  if (!brInst->isConditional()) {
    return;
  }

  for (size_t i{ BranchInst::LHSIdx }; i <= BranchInst::RHSIdx; i++) {
    if (auto op{ dynCastRef<Operand>(brInst->getOperand(i)) }) {
      auto reg{ chooseReadReg(op) };
      processSourceChoice(reg, op);

      brInst->setOperand(i, reg);
    }
  }
}

void Generator::visit(const Ref<ir::LoadInst>& loadInst) {
  if (auto op{ dynCastRef<Operand>(loadInst->getLoc()) }) {
    auto reg{ chooseReadReg(op) };
    processSourceChoice(reg, op);
    loadInst->setLoc(makeRef<RegisterLoc>(reg));
  }

  if (auto dstOp{ dynCastRef<Operand>(loadInst->getDestination()) }) {
    auto dstReg{ chooseDstReg(dstOp) };
    processDstChoice(dstReg, dstOp);
    loadInst->setDestination(dstReg);
  }
}

void Generator::visit(const Ref<ir::StoreInst>& storeInst) {
  if (auto sourceOp{ dynCastRef<Operand>(storeInst->getSource()) }) {
    storeInst->addMDNode(StoreSourceOperandMD{ sourceOp });
    auto reg{ chooseReadReg(sourceOp) };
    processSourceChoice(reg, sourceOp);
    storeInst->setSource(reg);
  }

  if (auto locOp{ dynCastRef<Operand>(storeInst->getLoc()) }) {
    auto locReg{ chooseReadReg(locOp) };
    processSourceChoice(locReg, locOp);
    storeInst->setLoc(makeRef<RegisterLoc>(locReg));
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
  int argRegN{ 0 };
  for (size_t i{ 0 }; i < callInst->getNumArgs(); i++) {
    auto arg{ callInst->getArg(i) };
    auto argOp{ dynCastRef<Operand>(arg) };

    if (argRegN == ArgRegisters.size()) {
      bort_assert(false,
                  "Too many arguments, memory args not implemented");
    }

    auto argReg{ RVMachineRegister::get(ArgRegisters.at(argRegN++)) };
    if (argOp) {
      processSourceChoice(argReg, argOp);
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

void Generator::visit(const Ref<ir::RetInst>& retInst) {

  markForRemoval(retInst);
  if (retInst->hasValue()) {
    auto reg{ RVMachineRegister::get(GPR::a0) };

    if (auto op{ dynCastRef<Operand>(retInst->getValue()) }) {
      if (!m_RegisterContent[reg].contains(op)) {
        generateLoad(op, reg);
      }
    } else {
      // it's immediate
      m_CurrentBBIter->insertBefore(
          m_CurrentInstIter,
          makeRef<MoveInst>(RVMachineRegister::get(GPR::a0),
                            retInst->getValue()));
    }

    retInst->setValue(std::move(reg));
  }

  auto& curFunc{ *m_CurrentFuncIter };
  auto* prologueEpilogueInfo{
    curFunc.getMDNode<RVFuncPrologueEpilogue>()
  };
  auto brToRet{ makeRef<BranchInst>() };
  brToRet->setTarget(prologueEpilogueInfo->EpilogueBB);
  m_CurrentBBIter->insertBefore(m_CurrentInstIter, std::move(brToRet));
}

void Generator::reinitDescriptors(const BasicBlock& bb, bool isEntry) {
  for (int i{ static_cast<int>(GPR::VALUE_REGS_START) };
       i != static_cast<int>(GPR::VALUE_REGS_END); i++) {
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

  if (isEntry) {
    for (auto&& argIt : m_CurrentFuncIter->getFunction()->getArgs() |
                            boost::adaptors::indexed()) {
      for (auto&& [op, _] : m_OperandLocs) {
        if (auto opVU{ dynCastRef<VariableUse>(op) }) {
          if (opVU->getVariable() == argIt.value()) {
            bort_assert(static_cast<size_t>(argIt.index()) <
                            ArgRegisters.size(),
                        "Memory args not implemented");
            auto argRegister{ RVMachineRegister::get(
                ArgRegisters.at(argIt.index())) };
            m_OperandLocs[op] = { makeRef<RegisterLoc>(argRegister) };
            m_RegisterContent[argRegister] = { op };
          }
        }
      }
    }
  }
}

void Generator::evaluateLocAddress(const Ref<ValueLoc>& loc,
                                   const RVMachineRegisterRef& dest) {
  if (auto stackLoc{ dynCastRef<StackLoc>(loc) }) {
    m_CurrentBBIter->insertBefore(
        m_CurrentInstIter,
        makeRef<OpInst>(TokenKind::Plus, dest,
                        RVMachineRegister::get(GPR::sp),
                        IntegralConstant::getInt(stackLoc->getOffset())));
    return;
  }

  bort_assert(false, fmt::format("Cant evaluate location address for: {}",
                                 loc->getName())
                         .c_str());
}

void Generator::addInstruction(const Ref<ir::Instruction>& inst) {
  m_CurrentBBIter->insertBefore(m_CurrentInstIter, inst);
  InstructionVisitorBase::genericVisit(inst);
}

} // namespace bort::codegen::rv
