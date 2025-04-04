#include "bort/Codegen/Utils.hpp"
#include "bort/Basic/Casts.hpp"
#include "bort/IR/BasicBlock.hpp"
#include "bort/IR/BranchInst.hpp"
#include "bort/IR/RetInst.hpp"
#include "bort/IR/Value.hpp"
#include <fmt/format.h>
#include <ranges>
#include <unordered_set>

using namespace bort::ir;

namespace bort::codegen {

auto getUniqueOperands(const IRFunction& func)
    -> std::vector<Ref<Operand>> {
  std::unordered_set<Ref<Operand>> uniqueOperands;
  std::vector<Ref<Operand>> result;
  for (const auto& block : func) {
    for (const auto& op : getUniqueOperands(block)) {
      if (!op) {
        // probably a constant
        continue;
      }
      auto [_, inserted]{ uniqueOperands.insert(op) };
      if (inserted) {
        result.push_back(op);
      }
    }
  }
  return result;
}

auto getUniqueOperands(const BasicBlock& block)
    -> std::unordered_set<Ref<Operand>> {
  std::unordered_set<Ref<Operand>> uniqueOperands;
  std::vector<Ref<Operand>> result;
  for (const auto& inst : block) {
    for (size_t i : std::views::iota(0U, inst->getNumOperands())) {
      auto op{ dynCastRef<Operand>(inst->getOperand(i)) };
      if (!op) {
        // probably a constant
        continue;
      }
      uniqueOperands.insert(op);
    }
  }
  return uniqueOperands;
}

auto FrameInfo::toString() const -> std::string {
  return fmt::format("frame_info .size={} .vars={}", Size, NumVariables);
}

auto isJumpInst(const Ref<ir::Instruction>& inst) -> bool {
  return isaRef<RetInst>(inst) || isaRef<BranchInst>(inst);
}

} // namespace bort::codegen
