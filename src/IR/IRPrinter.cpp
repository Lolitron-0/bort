#include "bort/IR/IRPrinter.hpp"
#include "bort/Basic/Casts.hpp"
#include "bort/IR/BranchInst.hpp"
#include "bort/IR/CallInst.hpp"
#include "bort/IR/Constant.hpp"
#include "bort/IR/LoadInst.hpp"
#include "bort/IR/MoveInst.hpp"
#include "bort/IR/OpInst.hpp"
#include "bort/IR/Register.hpp"
#include "bort/IR/RetInst.hpp"
#include "bort/IR/StoreInst.hpp"
#include "bort/IR/UnaryInst.hpp"
#include "bort/IR/Value.hpp"
#include "bort/Lex/Token.hpp"
#include "fmt/color.h"
#include <fmt/base.h>
#include <fmt/format.h>
#include <memory>

static constexpr auto s_InstColor(fmt::fg(fmt::color::blue_violet));
static constexpr auto s_RegColor(fmt::fg(fmt::color::pale_golden_rod));
static constexpr auto s_ConstColor(fmt::fg(fmt::color::pale_green));
static constexpr auto s_UnknownColor(fmt::fg(fmt::color::indian_red));
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
  res = res.substr(0, res.rfind(','));
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
  return fmt::format(s_UnknownColor, "?{}", val->getName());
}

template <typename T>
static auto styleInst(T&& name) {
  return fmt::styled(name, s_InstColor);
}

static constexpr cul::BiMap s_UnaryInstNames{ [](auto&& selector) {
  return selector.Case(TokenKind::Minus, "neg")
      .Case(TokenKind::Amp, "addr")
      .Case(TokenKind::Star, "deref");
} };

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

void IRPrinter::print(const Module& module) {
  for (auto&& func : module) {
    fmt::print(stderr, s_MDColor, "function {}\n", getMDClause(func));
    for (auto&& BB : func) {
      fmt::println(stderr, "{}:", BB.getName());
      for (auto&& inst : BB) {
        if (auto opInst{ dynCastRef<OpInst>(inst) }) {
          visit(opInst);
        } else if (auto unaryInst{ dynCastRef<UnaryInst>(inst) }) {
          visit(unaryInst);
        } else if (auto allocaInst{ dynCastRef<AllocaInst>(inst) }) {
          visit(allocaInst);
        } else if (auto moveInst{ dynCastRef<MoveInst>(inst) }) {
          fmt::print(stderr, "{} = {}",
                     formatValueColored(moveInst->getDestination()),
                     formatValueColored(moveInst->getSrc()));
        } else if (auto branchInst{ dynCastRef<BranchInst>(inst) }) {
          visit(branchInst);
        } else if (auto callInst{ dynCastRef<CallInst>(inst) }) {
          visit(callInst);
        } else if (auto retInst{ dynCastRef<RetInst>(inst) }) {
          visit(retInst);
        } else if (auto loadInst{ dynCastRef<LoadInst>(inst) }) {
          visit(loadInst);
        } else if (auto storeInst{ dynCastRef<StoreInst>(inst) }) {
          visit(storeInst);
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
             formatValueColored(opInst->getSrc()),
             formatValueColored(opInst->getSrc2()));
}

void bort::ir::IRPrinter::visit(const Ref<UnaryInst>& unaryInst) {
  bort_assert(s_UnaryInstNames.Find(unaryInst->getOp()).has_value(),
              "UnaryInst name not added");
  fmt::print(stderr, "{} = {} {}",
             formatValueColored(unaryInst->getDestination()),
             styleInst(s_UnaryInstNames.Find(unaryInst->getOp()).value()),
             formatValueColored(unaryInst->getSrc()));
}

void IRPrinter::visit(const Ref<AllocaInst>& allocaInst) {
  fmt::print(stderr, "{} = {} {}, {}",
             formatValueColored(allocaInst->getDestination()),
             styleInst("alloca"),
             formatValueColored(allocaInst->getElementSize()),
             formatValueColored(allocaInst->getNumElements()));
}

void IRPrinter::visit(const Ref<BranchInst>& branchInst) {
  static std::unordered_map<TokenKind, std::string> s_ModeNames{
    { TokenKind::NotEquals, "ne" }, { TokenKind::Equals, "eq" },
    { TokenKind::Less, "lt" },      { TokenKind::Greater, "gt" },
    { TokenKind::LessEqual, "le" }, { TokenKind::GreaterEqual, "ge" }
  };

  if (branchInst->isConditional()) {
    auto [lhs, rhs]{ branchInst->getOperands() };
    fmt::print(stderr, "{} {}, {}, {}",
               styleInst("b" + s_ModeNames.at(branchInst->getMode())),
               formatValueColored(lhs), formatValueColored(rhs),
               branchInst->getTarget()->getName());
  } else {
    fmt::print(stderr, "{} {}", styleInst("br"),
               branchInst->getTarget()->getName());
  }
}

void bort::ir::IRPrinter::visit(const Ref<CallInst>& callInst) {
  if (!callInst->isVoid()) {
    fmt::print(stderr,
               "{} = ", formatValueColored(callInst->getDestination()));
  }

  fmt::print(stderr, "{} {}", styleInst("call"),
             callInst->getFunction()->getName());

  for (size_t i{ 0 }; i < callInst->getNumArgs(); i++) {
    fmt::print(stderr, ", {}", formatValueColored(callInst->getArg(i)));
  }
}

void bort::ir::IRPrinter::visit(const Ref<RetInst>& retInst) {
  fmt::print(stderr, "{}", styleInst("ret"));
  if (retInst->hasValue()) {
    fmt::print(stderr, " {}", formatValueColored(retInst->getValue()));
  }
}

void bort::ir::IRPrinter::visit(const Ref<LoadInst>& loadInst) {
  fmt::print(stderr, "{} = {} {}, {}",
             formatValueColored(loadInst->getDestination()),
             styleInst("load"), formatValueColored(loadInst->getLoc()),
             formatValueColored(loadInst->getBytes()));
}

void bort::ir::IRPrinter::visit(const Ref<StoreInst>& storeInst) {
  fmt::print(stderr, "{} {}, {}, {}", styleInst("store"),
             formatValueColored(storeInst->getSource()),
             formatValueColored(storeInst->getLoc()),
             formatValueColored(storeInst->getBytes()));
}
