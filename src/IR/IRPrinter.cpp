#include "bort/IR/IRPrinter.hpp"
#include "bort/IR/BranchInst.hpp"
#include "bort/IR/Constant.hpp"
#include "bort/IR/IRCodegen.hpp"
#include "bort/IR/Instruction.hpp"
#include "bort/IR/MoveInst.hpp"
#include "bort/IR/OpInst.hpp"
#include "bort/IR/Value.hpp"
#include "bort/Lex/Token.hpp"
#include "fmt/color.h"
#include <fmt/base.h>
#include <fmt/format.h>
#include <memory>

static constexpr auto s_InstColor(fmt::fg(fmt::color::blue_violet));
static constexpr auto s_RegColor(fmt::fg(fmt::color::pale_golden_rod));
static constexpr auto s_ConstColor(fmt::fg(fmt::color::pale_green));

template <>
struct fmt::formatter<bort::ir::ValueRef>
    : fmt::formatter<std::string_view> {
  auto format(const bort::ir::ValueRef& val,
              format_context& ctx) const -> format_context::iterator {
    if (auto constant =
            std::dynamic_pointer_cast<bort::ir::IntConstant>(val)) {
      return formatter<std::string_view>::format(
          fmt::format(s_ConstColor, "{}", constant->getValue()), ctx);
    }
    if (auto inst =
            std::dynamic_pointer_cast<bort::ir::Instruction>(val)) {
      return formatter<std::string_view>::format(
          fmt::format("{}", inst->getDestination()), ctx);
    }
    return formatter<std::string_view>::format(
        fmt::format(s_RegColor, "%{}", val->getName()), ctx);
  }
};

namespace bort::ir {

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
  for (auto&& BB : module) {
    fmt::println(stderr, "{}:", BB.getName());
    for (auto&& inst : BB) {
      if (auto opInst{ std::dynamic_pointer_cast<OpInst>(inst) }) {
        visit(opInst);
      } else if (auto allocaInst{
                     std::dynamic_pointer_cast<AllocaInst>(inst) }) {
        visit(allocaInst);
      } else if (auto moveInst{
                     std::dynamic_pointer_cast<MoveInst>(inst) }) {
        fmt::println(stderr, "{} = {}", moveInst->getDestination(),
                     moveInst->getSrc());
      } else if (auto branchInst{
                     std::dynamic_pointer_cast<BranchInst>(inst) }) {
        visit(branchInst);
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

} // namespace bort::ir
