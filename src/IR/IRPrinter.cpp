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
    res += "!" + md->toString();
  }
  return fmt::format(s_MDColor, "[{}]", std::move(res));
}

template <>
struct fmt::formatter<bort::ir::ValueRef>
    : fmt::formatter<std::string_view> {
  auto format(const bort::ir::ValueRef& val,
              format_context& ctx) const -> format_context::iterator {
    if (auto constant =
            std::dynamic_pointer_cast<bort::ir::IntConstant>(val)) {
      return formatter<std::string_view>::format(
          fmt::format(s_ConstColor, "{}", constant->getValue()), ctx);
    } else if (bort::isaRef<bort::ir::Register>(val) ||
               bort::isaRef<bort::ir ::VariableUse>(val)) {
      return formatter<std::string_view>::format(
          fmt::format(s_RegColor, "%{}{}", val->getName(),
                      getMDClause(*val)),
          ctx);
    }
    bort_assert(false, "Can't format Value");
    return ctx.out();
  }
};

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
    fmt::print(stderr, s_MDColor, "{} ", getMDClause(func));
    for (auto&& BB : func) {
      fmt::println(stderr, "{}:", BB.getName());
      for (auto&& inst : BB) {
        if (auto opInst{ dynCastRef<OpInst>(inst) }) {
          visit(opInst);
        } else if (auto allocaInst{ dynCastRef<AllocaInst>(inst) }) {
          visit(allocaInst);
        } else if (auto moveInst{ dynCastRef<MoveInst>(inst) }) {
          fmt::println(stderr, "{} = {}", moveInst->getDestination(),
                       moveInst->getSrc());
        } else if (auto branchInst{ dynCastRef<BranchInst>(inst) }) {
          visit(branchInst);
        }
      }
    }
  }
}

void IRPrinter::visit(const Ref<OpInst>& opInst) {
  bort_assert(s_OpInstNames.Find(opInst->getOp()).has_value(),
              "OpInst name not added");
  fmt::println(stderr, "{} = {} {}, {}", opInst->getDestination(),
               styleInst(s_OpInstNames.Find(opInst->getOp()).value()),
               opInst->getSrc1(), opInst->getSrc2());
}

void IRPrinter::visit(const Ref<AllocaInst>& allocaInst) {
  fmt::println(stderr, "{} = {} {}, {}", allocaInst->getDestination(),
               styleInst("alloca"), allocaInst->getElementSize(),
               allocaInst->getNumElements());
}

void IRPrinter::visit(const Ref<BranchInst>& branchInst) {
  if (branchInst->isConditional()) {
    fmt::println(stderr, "{} {}, {}", styleInst("br"),
                 branchInst->getCondition(),
                 branchInst->getTarget()->getName());
  } else {
    fmt::println(stderr, "{} {}", styleInst("br"),
                 branchInst->getTarget()->getName());
  }
}
