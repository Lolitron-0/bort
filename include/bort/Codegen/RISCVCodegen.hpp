#pragma once
#include "bort/CLI/CLIOptions.hpp"
#include "bort/Codegen/MachineRegister.hpp"
#include "bort/IR/BasicBlock.hpp"
#include "bort/IR/Module.hpp"
#include "bort/IR/Value.hpp"
#include "bort/IR/ValueLoc.hpp"
#include <unordered_set>
#include <utility>

namespace bort::codegen::rv {

enum class GPR : uint8_t {
  t0,
  t1,
  t2,
  t3,
  t4,
  t5,
  t6,
  s0,
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
  a0,
  a1,
  a2,
  a3,
  a4,
  a5,
  a6,
  a7,
  COUNT
};

auto GPRToString(GPR gpr) -> std::string_view;

using RVMachineRegister    = MachineRegister<GPR, GPRToString>;
using RVMachineRegisterRef = Ref<RVMachineRegister>;

/// General note: for this type of IR, graph-coloring algorithm would be
/// more preferable, but firstly I decided to quickly implement something
/// more simple
class Generator {
public:
  Generator(CLIOptions cliOptions, ir::Module& module)
      : m_Module{ module },
        m_CLIOptions{ std::move(cliOptions) } {
  }

  void generate();

private:
  void processInst(ir::InstIter it, ir::BasicBlock& bb);

  auto chooseUseReg(Ref<ir::Operand> op, ir::InstIter it,
                    std::initializer_list<GPR> ignoreRegisters = {})
      -> RVMachineRegisterRef;
  void reinitDescriptors(const ir::BasicBlock& bb);
  void assignLocalVariablesOffsets();

  ir::Module& m_Module;
  CLIOptions m_CLIOptions;
  std::unordered_map<RVMachineRegisterRef,
                     std::unordered_set<Ref<ir::Operand>>>
      m_RegisterContent;
  std::unordered_map<Ref<ir::Operand>, std::unordered_set<Ref<ir::ValueLoc>>>
      m_OperandLocs;
};

} // namespace bort::codegen::rv
