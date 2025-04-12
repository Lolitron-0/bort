#include "bort/Codegen/RISCVPrinter.hpp"
#include "bort/Basic/Assert.hpp"
#include "bort/Basic/Casts.hpp"
#include "bort/Codegen/Intrinsics.hpp"
#include "bort/Codegen/RISCVCodegen.hpp"
#include "bort/Codegen/ValueLoc.hpp"
#include "bort/Frontend/Type.hpp"
#include "bort/IR/BranchInst.hpp"
#include "bort/IR/Constant.hpp"
#include "bort/IR/GlobalValue.hpp"
#include "bort/IR/MoveInst.hpp"
#include "bort/IR/StoreInst.hpp"
#include "bort/IR/Value.hpp"
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <ostream>

using namespace bort::ir;
using namespace bort;

namespace bort::codegen::rv {

static auto formatValueLoc(const Ref<ValueLoc>& VL) -> std::string {
  if (auto SL{ dynCastRef<StackLoc>(VL) }) {
    return fmt::format("{}(sp)", SL->getOffset());
  }

  if (auto GL{ dynCastRef<GlobalLoc>(VL) }) {
    return fmt::format("{}", GL->getGV()->getName());
  }

  if (auto RL{ dynCastRef<RegisterLoc>(VL) }) {
    return fmt::format("({})", RL->getRegister()->getName());
  }

  bort_assert(false, "Unhandled ValueLoc");
  return {};
}

auto formatMachineValue(const ValueRef& val) -> std::string {
  auto defaultFmt{ Value::formatValue(val) };
  if (defaultFmt.has_value()) {
    return defaultFmt.value();
  }
  if (auto rvReg{
          dynCastRef<bort::codegen::rv::RVMachineRegister>(val) }) {
    return std::string{ GPRToString(rvReg->getGPRId()) };
  }
  if (auto VL{ dynCastRef<ValueLoc>(val) }) {
    return formatValueLoc(VL);
  }

  bort_assert(false, "Can't format Value in codegen");
  return "";
}

struct GlobalValueInitializerFormatter {

  auto visit(GlobalInitializer* /*val*/) -> std::string {
    return {};
  }

  auto visit(GlobalVariable* val) -> std::string {
    std::string initializer{};
    if (auto constant{
            dynCastRef<IntegralConstant>(val->getInitializer()) }) {
      initializer = fmt::format("{}", constant->getValue());
    } else if (auto initList{ dynCastRef<GlobalInitializer>(
                   val->getInitializer()) }) {
      std::string delim{ ", " };
      for (auto&& val : initList->getValues()) {
        initializer += fmt::format("{}{}", delim, val->getValue());
      }
      initializer.erase(0, delim.size());
    }
    return fmt::format("{}: {} {}", val->getName(),
                       formatGlobalVarTypeLabel(val->getType()),
                       std::move(initializer));
  }

private:
  static auto formatGlobalVarTypeLabel(const TypeRef& type)
      -> std::string {
    if (type->getKind() == TypeKind::Int ||
        type->getKind() == TypeKind::Pointer) {
      return ".word";
    }
    if (type->getKind() == TypeKind::Char) {
      return ".byte";
    }

    if (auto arrayType{ dynCastRef<ArrayType>(type) }) {
      return formatGlobalVarTypeLabel(arrayType->getBaseType());
    }

    bort_assert(false, "Unhandled GV type label");
    return {};
  }
};

void Printer::run(ir::Module& module) {
  printHeader(module);
  for (auto&& func : module) {
    bool firstBlock{ true };
    const auto* PE{ func.getMDNode<RVFuncPrologueEpilogue>() };
    fmt::println(m_Stream, "{}:", func.begin()->getName());
    /// @todo this is bad af, better make another structure
    fmt::print(m_Stream, "{}", PE->Prologue);
    for (auto&& BB : func) {
      if (!firstBlock) {
        fmt::println(m_Stream, "{}:", BB.getName());
      }
      firstBlock = false;
      for (auto&& inst : BB) {
        genericVisit(inst);
      }
    }
    fmt::print(m_Stream, "{}", PE->Epilogue);
  }
}

void Printer::visit(const Ref<ir::OpInst>& opInst) {
  /// @todo MD node for immediatness, signedness, etc...
  auto* II{ opInst->getMDNode<RVInstInfo>() };
  auto* OAI{ opInst->getMDNode<RVOpAdditionalInfo>() };
  auto opName{ II->InstName };
  auto src1Reg{ dynCastRef<RVMachineRegister>(opInst->getSrc()) };
  auto src2Reg{ dynCastRef<RVMachineRegister>(opInst->getSrc2()) };
  auto dstReg{ dynCastRef<RVMachineRegister>(opInst->getDestination()) };
  if (OAI && OAI->IsSingleOp) {
    fmt::println(m_Stream, "{} {}, {}", opName,
                 formatMachineValue(opInst->getDestination()),
                 formatMachineValue(opInst->getSrc()));
  } else {
    fmt::println(m_Stream, "{} {}, {}, {}", opName,
                 formatMachineValue(opInst->getDestination()),
                 formatMachineValue(opInst->getSrc()),
                 formatMachineValue(opInst->getSrc2()));
  }
}

void Printer::visit(const Ref<ir::UnaryInst>& unaryInst) {
  auto* II{ unaryInst->getMDNode<RVInstInfo>() };
  fmt::println(m_Stream, "{} {}, {}", II->InstName,
               formatMachineValue(unaryInst->getDestination()),
               formatMachineValue(unaryInst->getSrc()));
}

void Printer::visit(const Ref<ir::BranchInst>& brInst) {
  auto* II{ brInst->getMDNode<RVInstInfo>() };
  auto* BRI{ brInst->getMDNode<RVBranchInfo>() };

  if (brInst->isConditional()) {
    if (BRI && BRI->NoRHS) {
      fmt::println(m_Stream, "{} {}, {}", II->InstName,
                   formatMachineValue(brInst->getLHS()),
                   brInst->getTarget()->getName());
    } else {
      fmt::println(m_Stream, "{} {}, {}, {}", II->InstName,
                   formatMachineValue(brInst->getLHS()),
                   formatMachineValue(brInst->getRHS()),
                   brInst->getTarget()->getName());
    }
  } else {
    fmt::println(m_Stream, "{} {}", II->InstName,
                 brInst->getTarget()->getName());
  }
}

void Printer::visit(const Ref<RARSMacroCallInst>& macroInst) {
  fmt::print(m_Stream, "{}",
             intrinsics::getMacroName(macroInst->getMacroID()));
  for (auto&& arg : macroInst->getArgs()) {
    fmt::print(m_Stream, " {}", formatMachineValue(arg));
  }
  fmt::print(m_Stream, "\n");
}

void Printer::visit(const Ref<LoadInst>& loadInst) {
  auto* II{ loadInst->getMDNode<RVInstInfo>() };
  fmt::println(m_Stream, "{} {}, {}", II->InstName,
               formatMachineValue(loadInst->getDestination()),
               formatMachineValue(loadInst->getLoc()));
}

void Printer::visit(const Ref<ir::MoveInst>& mvInst) {
  auto* II{ mvInst->getMDNode<RVInstInfo>() };
  fmt::println(m_Stream, "{} {}, {}", II->InstName,
               formatMachineValue(mvInst->getDestination()),
               formatMachineValue(mvInst->getSrc()));
}

void Printer::visit(const Ref<StoreInst>& storeInst) {
  auto* II{ storeInst->getMDNode<RVInstInfo>() };
  auto* storeTmpRegMD{ storeInst->getMDNode<RVStoreTmpReg>() };

  std::string tmpRegStr{};
  if (storeTmpRegMD) {
    tmpRegStr = ", " + formatMachineValue(storeTmpRegMD->Reg);
  }

  fmt::println(m_Stream, "{} {}, {}{}", II->InstName,
               formatMachineValue(storeInst->getSource()),
               formatMachineValue(storeInst->getLoc()),
               std::move(tmpRegStr));
}

void Printer::visit(const Ref<ir::CallInst>& callInst) {
  auto* II{ callInst->getMDNode<RVInstInfo>() };
  // function name being actual function entry block needs checking but
  // ...
  fmt::println(m_Stream, "{} {}", II->InstName,
               callInst->getFunction()->getName());
}

void Printer::printHeader(const Module& module) {
  const auto* macros{ module.getMDNode<RARSMacroDefinitions>() };
  bort_assert(macros, "Module has no macro definitions MD");
  for (auto&& def : macros->Macros) {
    fmt::println(m_Stream, "{}\n", def);
  }

  fmt::println(m_Stream, ".data");

  for (auto&& GV : module.getGlobals()) {
    fmt::println(m_Stream, "{}",
                 GV->accept(GlobalValueInitializerFormatter{}));
  }

  fmt::println(m_Stream, R"(.globl main
.text
)");
}

} // namespace bort::codegen::rv
