#include "bort/Codegen/Utils.hpp"
#include "bort/Basic/Casts.hpp"
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
    for (const auto& inst : block) {
      for (size_t i : std::views::iota(0U, inst->getNumOperands())) {
        auto op{ dynCastRef<Operand>(inst->getOperand(i)) };
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
  }
  return result;
}
auto FrameInfo::toString() const -> std::string {
  return fmt::format("frame_info .size={} .vars={}", Size, NumVariables);
}
} // namespace bort::codegen
