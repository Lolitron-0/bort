#include "bort/IR/IRPrinter.hpp"
#include "bort/Basic/Casts.hpp"
#include "bort/IR/BranchInst.hpp"
#include "bort/IR/Constant.hpp"
#include "bort/IR/MoveInst.hpp"
#include "bort/IR/OpInst.hpp"
#include "bort/IR/Register.hpp"
#include "bort/IR/Value.hpp"
#include "bort/Lex/Token.hpp"
#include "fmt/color.h"
#include <fmt/base.h>
#include <fmt/format.h>
#include <memory>

static constexpr auto s_InstColor(fmt::fg(fmt::color::blue_violet));
static constexpr auto s_RegColor(fmt::fg(fmt::color::pale_golden_rod));
static constexpr auto s_ConstColor(fmt::fg(fmt::color::pale_green));
static constexpr auto s_MDColor(fmt::fg(fmt::color::slate_gray));

using namespace bort::ir;
using namespace bort;

static auto getMDClause(const Value& val) -> std::string {
  if (val.getMDRange().empty()) {
    return "";
  }

  std::string res;
  for (auto&& md : val.getMDRange()) {
    res += "!" + md->toString() + ", ";
  }
  return fmt::format(s_MDColor, "[{}]", std::move(res));
}

auto formatValueColored(const bort::ir::ValueRef& val) -> std::string {
  if (auto constant = dynCastRef<bort::ir::IntConstant>(val)) {
    return fmt::format(s_ConstColor, "{}", constant->getValue());
  }
  if (bort::isaRef<bort::ir::Register>(val) ||
      bort::isaRef<bort::ir ::VariableUse>(val)) {
    return fmt::format(s_RegColor, "%{}{}", val->getName(),
                       getMDClause(*val));
  }
  // bort_assert(false, "Can't format Value");
  return "UNK";
}

template <typename T>
static auto styleInst(T&& name) {
  return fmt::styled(name, s_InstColor);
}

static constexpr cul::BiMap s_OpInstNames{ [](auto&& selector) {
  return selector.Case(TokenKind::Plus, "add")
      .Case(TokenKind::Minus, "sub")
      .Case(TokenKind::Star, "mul")
      .Case(TokenKind::Div, "div")
      .Case(TokenKind::Less, "slt")
      .Case(TokenKind::Greater, "sgt");
} };

void IRPrinter::print(const Module& module) {
  for (auto&& func : module) {
    fmt::print(stderr, s_MDColor, "function {}\n", getMDClause(func));
    for (auto&& BB : func) {
      fmt::println(stderr, "{}:", BB.getName());
      for (auto&& inst : BB) {
        if (auto opInst{ dynCastRef<OpInst>(inst) }) {
          visit(opInst);
        } else if (auto allocaInst{ dynCastRef<AllocaInst>(inst) }) {
          visit(allocaInst);
        } else if (auto moveInst{ dynCastRef<MoveInst>(inst) }) {
          fmt::print(stderr, "{} = {}",
                     formatValueColored(moveInst->getDestination()),
                     formatValueColored(moveInst->getSrc()));
        } else if (auto branchInst{ dynCastRef<BranchInst>(inst) }) {
          visit(branchInst);
        } else {
          continue;
        }
        fmt::print(stderr, "; {}\n", getMDClause(*inst));
      }
    }
  }
}

void IRPrinter::visit(const Ref<OpInst>& opInst) {
  bort_assert(s_OpInstNames.Find(opInst->getOp()).has_value(),
              "OpInst name not added");
  fmt::print(stderr, "{} = {} {}, {}",
             formatValueColored(opInst->getDestination()),
             styleInst(s_OpInstNames.Find(opInst->getOp()).value()),
             formatValueColored(opInst->getSrc1()),
             formatValueColored(opInst->getSrc2()));
}

void IRPrinter::visit(const Ref<AllocaInst>& allocaInst) {
  fmt::print(stderr, "{} = {} {}, {}",
             formatValueColored(allocaInst->getDestination()),
             styleInst("alloca"),
             formatValueColored(allocaInst->getElementSize()),
             formatValueColored(allocaInst->getNumElements()));
}

void IRPrinter::visit(const Ref<BranchInst>& branchInst) {
  if (branchInst->isConditional()) {
    fmt::print(stderr, "{} {}, {}", styleInst("br"),
               formatValueColored(branchInst->getCondition()),
               branchInst->getTarget()->getName());
  } else {
    fmt::print(stderr, "{} {}", styleInst("br"),
               branchInst->getTarget()->getName());
  }
}
