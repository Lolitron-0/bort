#include "bort/IR/IRPrinter.hpp"
#include "bort/Basic/Casts.hpp"
#include "bort/IR/AllocaInst.hpp"
#include "bort/IR/BranchInst.hpp"
#include "bort/IR/CallInst.hpp"
#include "bort/IR/Constant.hpp"
#include "bort/IR/GlobalValue.hpp"
#include "bort/IR/LoadInst.hpp"
#include "bort/IR/MoveInst.hpp"
#include "bort/IR/OpInst.hpp"
#include "bort/IR/Register.hpp"
#include "bort/IR/RetInst.hpp"
#include "bort/IR/StoreInst.hpp"
#include "bort/IR/UnaryInst.hpp"
#include "bort/IR/Value.hpp"
#include "bort/IR/VariableUse.hpp"
#include "bort/Lex/Token.hpp"
#include <fmt/base.h>
#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <memory>

static constexpr auto s_DefaultStyle(fmt::fg(fmt::color::white));
static constexpr auto s_InstructionStyle(
    fmt::fg(fmt::color::blue_violet));
static constexpr auto s_RegisterStyle(
    fmt::fg(fmt::color::pale_golden_rod));
static constexpr auto s_GlobalStyle(s_RegisterStyle |
                                    fmt::emphasis::italic);
static constexpr auto s_ConstantStyle(fmt::fg(fmt::color::pale_green));
static constexpr auto s_UnknownStyle(fmt::fg(fmt::color::indian_red));
static constexpr auto s_MDStyle(fmt::fg(fmt::color::slate_gray));

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
  return fmt::format(s_MDStyle, "[{}]", std::move(res));
}

auto formatValueColored(const bort::ir::ValueRef& val) -> std::string {
  if (auto constant{ dynCastRef<bort::ir::IntegralConstant>(val) }) {
    return fmt::format(s_ConstantStyle, "{}", constant->getValue());
  }
  if (auto GV{ dynCastRef<bort::ir::GlobalValue>(val) }) {
    return fmt::format(s_GlobalStyle, "%{}", GV->getName());
  }
  if (bort::isaRef<bort::ir::Register>(val) ||
      bort::isaRef<bort::ir ::VariableUse>(val)) {
    return fmt::format(s_RegisterStyle, "%{}{}", val->getName(),
                       getMDClause(*val));
  }
  return fmt::format(s_UnknownStyle, "?{}",
                     val ? val->getName() : "null");
}

static auto formatFuncSignature(const IRFunction& func) -> std::string {
  std::string args{};
  std::string delim{ ", " };
  for (auto&& arg : func.getFunction()->getArgs()) {
    args += delim + arg->getType()->toString() + " " +
            formatValueColored(VariableUse::get(arg));
  }
  args.erase(0, delim.size());
  return fmt::format(s_DefaultStyle, "{}({})", func.getName(),
                     std::move(args));
}

struct GlobalValueInitializerFormatter {
  auto visit(GlobalArray* val) -> std::string {
    std::string values{};
    std::string delim{ ", " };
    for (auto&& n : val->getValues()) {
      values += fmt::format(s_ConstantStyle, "{}", n->getValue()) + delim;
    }

    if (!values.empty()) {
      values.erase(values.size() - delim.size());
    }
    return fmt::format("{} = {{{}}}", formatValueNameColored(val),
                       values);
  }

private:
  static auto formatValueNameColored(GlobalValue* val) -> std::string {
    return fmt::format(s_GlobalStyle, "{}", val->getName());
  }
};

template <typename T>
static auto styleInst(T&& name) {
  return fmt::styled(name, s_InstructionStyle);
}

static constexpr cul::BiMap s_UnaryInstNames{ [](auto&& selector) {
  return selector.Case(TokenKind::Minus, "neg")
      .Case(TokenKind::Amp, "addr")
      .Case(TokenKind::Star, "deref")
      .Case(TokenKind::PlusPlus, "inc")
      .Case(TokenKind::MinusMinus, "dec")
      .Case(TokenKind::Not, "not");
} };

static constexpr cul::BiMap s_OpInstNames{ [](auto&& selector) {
  return selector.Case(TokenKind::Plus, "add")
      .Case(TokenKind::Minus, "sub")
      .Case(TokenKind::Star, "mul")
      .Case(TokenKind::Div, "div")
      .Case(TokenKind::Less, "slt")
      .Case(TokenKind::Greater, "sgt")
      .Case(TokenKind::GreaterEqual, "sge")
      .Case(TokenKind::LessEqual, "sle")
      .Case(TokenKind::Equals, "seq")
      .Case(TokenKind::NotEquals, "sne")
      .Case(TokenKind::Amp, "and")
      .Case(TokenKind::Pipe, "or")
      .Case(TokenKind::Xor, "xor");
} };

namespace bort {

template <>
struct VisitorTraits<IRPrinter> {
  static constexpr bool isRefBased      = true;
  static constexpr bool ignoreUnhandled = true;
};

} // namespace bort

void IRPrinter::print(const Module& module) {
  for (auto&& GV : module.getGlobals()) {
    fmt::println(stderr, "{}",
                 GV->accept(GlobalValueInitializerFormatter{}));
  }
  fmt::print(stderr, "\n");

  for (auto&& func : module) {
    fmt::print(stderr, s_MDStyle, "function {} {}\n",
               formatFuncSignature(func), getMDClause(func));
    for (auto&& BB : func) {
      fmt::println(stderr, "{}:", BB.getName());
      for (auto&& inst : BB) {
        VisitDispatcher<IRPrinter, Instruction, OpInst, UnaryInst,
                        AllocaInst, MoveInst, GepInst, BranchInst,
                        CallInst, RetInst, LoadInst,
                        StoreInst>::dispatch(inst, *this);
        fmt::print(stderr, "; {}\n", getMDClause(*inst));
      }
    }
    fmt::print(stderr, "\n");
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

void bort::ir::IRPrinter::visit(const Ref<GepInst>& gepInst) {
  fmt::print(stderr, "{} = {} {}, {}",
             formatValueColored(gepInst->getDestination()),
             styleInst("gep"), formatValueColored(gepInst->getBasePtr()),
             formatValueColored(gepInst->getIndex()));
}

void IRPrinter::visit(const Ref<AllocaInst>& allocaInst) {
  fmt::print(stderr, "{} = {} {}, {}",
             formatValueColored(allocaInst->getDestination()),
             styleInst("alloca"),
             formatValueColored(allocaInst->getElementSize()),
             formatValueColored(allocaInst->getNumElements()));
}

void bort::ir::IRPrinter::visit(const Ref<MoveInst>& moveInst) {
  fmt::print(stderr, "{} = {}",
             formatValueColored(moveInst->getDestination()),
             formatValueColored(moveInst->getSrc()));
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
