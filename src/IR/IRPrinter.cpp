#include "bort/IR/IRPrinter.hpp"
#include "bort/IR/Constant.hpp"
#include "bort/IR/Instruction.hpp"
#include "bort/IR/OpInst.hpp"
#include "bort/IR/Value.hpp"
#include "bort/Lex/Token.hpp"
#include <fmt/base.h>
#include <fmt/format.h>

template <>
struct fmt::formatter<bort::ir::ValueRef>
    : fmt::formatter<std::string_view> {
  auto format(bort::ir::ValueRef val,
              format_context& ctx) const -> format_context::iterator {
    if (auto constant =
            std::dynamic_pointer_cast<bort::ir::IntConstant>(val)) {
      return formatter<std::string_view>::format(
          std::to_string(constant->getValue()), ctx);
    } else if (auto inst =
                   std::dynamic_pointer_cast<bort::ir::Instruction>(
                       val)) {
      return formatter<std::string_view>::format(
          fmt::format("{}", inst->getDestination()), ctx);
    }
    return formatter<std::string_view>::format(
        fmt::format("%{}", val->getName()), ctx);
  }
};

namespace bort::ir {

static constexpr cul::BiMap s_OpInstNames{ [](auto&& selector) {
  return selector.Case(TokenKind::Plus, "add")
      .Case(TokenKind::Minus, "sub")
      .Case(TokenKind::Star, "mul")
      .Case(TokenKind::Div, "div");
} };

void IRPrinter::print(const std::vector<Ref<Instruction>>& instructions) {
  for (auto&& inst : instructions) {
    if (auto opInst{ std::dynamic_pointer_cast<OpInst>(inst) }) {
      visit(opInst);
    }
  }
}

void IRPrinter::visit(const Ref<OpInst>& opInst) {
  bort_assert(s_OpInstNames.Find(opInst->getOp()).has_value(),
              "OpInst name not added");
  fmt::println(stderr, "{} = {} {} {}", opInst->getDestination(),
               s_OpInstNames.Find(opInst->getOp()).value(),
               opInst->getSrc1(), opInst->getSrc2());
}

} // namespace bort::ir
