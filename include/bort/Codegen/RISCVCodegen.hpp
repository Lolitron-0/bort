#pragma once
#include "bort/CLI/CLIOptions.hpp"
#include "bort/Codegen/InstructionVisitorBase.hpp"
#include "bort/Codegen/MachineRegister.hpp"
#include "bort/Codegen/ValueLoc.hpp"
#include "bort/IR/AllocaInst.hpp"
#include "bort/IR/BasicBlock.hpp"
#include "bort/IR/BranchInst.hpp"
#include "bort/IR/CallInst.hpp"
#include "bort/IR/Instruction.hpp"
#include "bort/IR/LoadInst.hpp"
#include "bort/IR/Module.hpp"
#include "bort/IR/MoveInst.hpp"
#include "bort/IR/OpInst.hpp"
#include "bort/IR/StoreInst.hpp"
#include "bort/IR/UnaryInst.hpp"
#include "bort/IR/Value.hpp"
#include <functional>
#include <map>
#include <unordered_set>
#include <utility>

namespace bort::codegen::rv {

enum class GPR : uint8_t {
  VALUE_REGS_START,
  t0 = VALUE_REGS_START,
  t1,
  t2,
  t3,
  t4,
  t5,
  t6,
  a7,
  a6,
  a5,
  a4,
  a3,
  a2,
  a1,
  a0,
  // s0 - reserved
  s1,
  s2,
  s3,
  s4,
  s5,
  s6,
  s7,
  s8,
  s9,
  s10,
  s11,
  VALUE_REGS_END,
  SPECIAL_REGS_START,
  sp = SPECIAL_REGS_START,
  ra,
  fp,
  SPECIAL_REGS_END,
  NUM_REGS = (VALUE_REGS_END - VALUE_REGS_START) +
             (SPECIAL_REGS_END - SPECIAL_REGS_START)
};

auto GPRToString(GPR gpr) -> std::string_view;

struct RVFuncPrologueEpilogue : public ir::Metadata {
  std::string Prologue;
  std::string Epilogue;
  ir::BasicBlock* EpilogueBB;

  [[nodiscard]] auto toString() const -> std::string override;
};

struct RVInstInfo final : public ir::Metadata {
  explicit RVInstInfo(std::string instName)
      : InstName{ std::move(instName) } {
  }

  [[nodiscard]] auto toString() const -> std::string override;

  std::string InstName;
};

struct RVBranchInfo final : public ir::Metadata {
  explicit RVBranchInfo(bool isSingleOp)
      : IsSingleOp{ isSingleOp } {
  }

  [[nodiscard]] auto toString() const -> std::string override;

  bool IsSingleOp;
};

class RVMachineRegister final : public MachineRegister {
public:
  static auto get(GPR gprId) -> Ref<RVMachineRegister>;

  [[nodiscard]] auto getGPRId() const -> GPR {
    return static_cast<GPR>(MachineRegister::getGPRId());
  }

private:
  RVMachineRegister(GPR gprId, std::string name)
      : MachineRegister(static_cast<int>(gprId), std::move(name)) {
  }
};
using RVMachineRegisterRef = Ref<RVMachineRegister>;

/// General note: for this type of IR, graph-coloring algorithm would be
/// more preferable, but firstly I decided to quickly implement something
/// more simple
class Generator : public InstructionVisitorBase {
public:
  Generator(CLIOptions cliOptions, ir::Module& module)
      : m_Module{ module },
        m_CLIOptions{ std::move(cliOptions) } {
  }

  void generate();

private:
  void generateLoad(const Ref<ir::Operand>& op,
                    const RVMachineRegisterRef& reg);
  void generateStore(const Ref<ir::Operand>& op,
                     const Ref<MachineRegister>& reg,
                     const Ref<ValueLoc>& loc);

  void processInst();
  void visit(const Ref<ir::OpInst>& opInst) override;
  void visit(const Ref<ir::UnaryInst>& unaryInst) override;
  void visit(const Ref<ir::BranchInst>& brInst) override;
  void visit(const Ref<ir::CallInst>& callInst) override;
  void visit(const Ref<ir::RetInst>& retInst) override;
  void visit(const Ref<ir::MoveInst>& mvInst) override;
  void visit(const Ref<ir::LoadInst>& loadInst) override;
  void visit(const Ref<ir::StoreInst>& storeInst) override;

  auto tryFindRegisterWithOperand(const Ref<ir::Operand>& op)
      -> std::optional<RVMachineRegisterRef>;
  auto chooseReadReg(const Ref<ir::Operand>& op) -> RVMachineRegisterRef;
  auto chooseDstReg(const Ref<ir::Operand>& op) -> RVMachineRegisterRef;
  void processSourceChoice(const RVMachineRegisterRef& reg,
                           const Ref<ir::Operand>& op);
  void processDstChoice(const RVMachineRegisterRef& reg,
                        const Ref<ir::Operand>& op);
  void reinitDescriptors(const ir::BasicBlock& bb);
  void addInstruction(const Ref<ir::Instruction>& inst);
  void assignLocalOperandsOffsets();
  auto getOperandRegisterMemoryLocs(const Ref<ir::Operand>& op) const
      -> std::pair<Ref<RegisterLoc>, Ref<ValueLoc>>;

  using SpillFilter = std::function<bool(const Ref<ir::Operand>&)>;
  void spillIf(const SpillFilter& filter = [](const auto&) {
    return true;
  });
  void evaluateLocAddress(const Ref<ValueLoc>& loc,
                          const RVMachineRegisterRef& dest);
  void markForRemoval(const Ref<ir::Instruction>& inst) const;

  ir::Module& m_Module;
  CLIOptions m_CLIOptions;
  std::map<RVMachineRegisterRef, std::unordered_set<Ref<ir::Operand>>>
      m_RegisterContent;
  std::unordered_map<Ref<ir::Operand>, std::unordered_set<Ref<ValueLoc>>>
      m_OperandLocs;
};

} // namespace bort::codegen::rv
