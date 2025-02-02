#include "bort/Codegen/RISCVPrinter.hpp"
#include "bort/Basic/Assert.hpp"
#include "bort/Basic/Casts.hpp"
#include "bort/Codegen/RISCVCodegen.hpp"
#include "bort/Codegen/StoreInst.hpp"
#include "bort/Codegen/ValueLoc.hpp"
#include "bort/IR/MoveInst.hpp"
#include "bort/IR/Value.hpp"
#include <fmt/format.h>
#include <fmt/ostream.h>

using namespace bort::ir;
using namespace bort;

namespace bort::codegen::rv {

auto formatMachineValue(const ValueRef& val) -> std::string {
  auto defaultFmt{ Value::formatValue(val) };
  if (defaultFmt.has_value()) {
    return defaultFmt.value();
  }
  if (auto rvReg{
          dynCastRef<bort::codegen::rv::RVMachineRegister>(val) }) {
    return std::string{ GPRToString(rvReg->getGPRId()) };
  }

  bort_assert(false, "Can't format Value in codegen");
  return "";
}

static auto formatValueLoc(const Ref<ValueLoc>& VL) -> std::string {
  if (auto SL{ dynCastRef<StackLoc>(VL) }) {
    return fmt::format("{}(sp)", SL->getOffset());
  }

  bort_assert(false, "Unhandled ValueLoc");
  return {};
}

void Printer::run(ir::Module& module) {
  printHeader();
  for (auto&& func : module) {
    bool firstBlock{ true };
    const auto* AC{ func.getMDNode<RVFuncAdditionalCode>() };
    fmt::println(m_Stream, "{}:", func.begin()->getName());
    /// @todo this is bad af, better make another structure
    fmt::print(m_Stream, "{}", AC->Prologue);
    for (auto&& BB : func) {
      if (!firstBlock) {
        fmt::println(m_Stream, "{}:", BB.getName());
      }
      firstBlock = false;
      for (auto&& inst : BB) {
        genericVisit(inst);
      }
    }
    fmt::print(m_Stream, "{}", AC->Epilogue);
  }
}

void Printer::visit(const Ref<ir::OpInst>& opInst) {
  /// @todo MD node for immediatness, signedness, etc...
  auto* II{ opInst->getMDNode<RVInstInfo>() };
  auto opName{ II->InstName };
  auto src1Reg{ dynCastRef<RVMachineRegister>(opInst->getSrc1()) };
  auto src2Reg{ dynCastRef<RVMachineRegister>(opInst->getSrc2()) };
  auto dstReg{ dynCastRef<RVMachineRegister>(opInst->getDestination()) };
  fmt::println(m_Stream, "{} {}, {}, {}", opName,
               formatMachineValue(opInst->getDestination()),
               formatMachineValue(opInst->getSrc1()),
               formatMachineValue(opInst->getSrc2()));
}

void Printer::visit(const Ref<ir::BranchInst>& brInst) {
  auto* II{ brInst->getMDNode<RVInstInfo>() };

  if (brInst->isConditional()) {
    fmt::println(m_Stream, "{} {}, {}", II->InstName,
                 formatMachineValue(brInst->getCondition()),
                 brInst->getTarget()->getName());
  } else {
    fmt::println(m_Stream, "{} {}", II->InstName,
                 brInst->getTarget()->getName());
  }
}

void Printer::visit(const Ref<LoadInst>& loadInst) {
  auto* II{ loadInst->getMDNode<RVInstInfo>() };
  fmt::println(m_Stream, "{} {}, {}", II->InstName,
               formatMachineValue(loadInst->getDestination()),
               formatValueLoc(loadInst->getLoc()));
}

void Printer::visit(const Ref<ir::MoveInst>& mvInst) {
  auto* II{ mvInst->getMDNode<RVInstInfo>() };
  fmt::println(m_Stream, "{} {}, {}", II->InstName,
               formatMachineValue(mvInst->getDestination()),
               formatMachineValue(mvInst->getSrc()));
}

void Printer::visit(const Ref<StoreInst>& storeInst) {
  auto* II{ storeInst->getMDNode<RVInstInfo>() };
  fmt::println(m_Stream, "{} {}, {}", II->InstName,
               formatMachineValue(storeInst->getSource()),
               formatValueLoc(storeInst->getLoc()));
}

void Printer::visit(const Ref<ir::CallInst>& callInst) {
  auto* II{ callInst->getMDNode<RVInstInfo>() };
  // function name being actual function entry block needs checking but
  // ...
  fmt::println(m_Stream, "{} {}", II->InstName,
               callInst->getFunction()->getName());
}

void Printer::printHeader() {
  fmt::println(m_Stream, R"(
.globl main
.text
)");
}

} // namespace bort::codegen::rv
