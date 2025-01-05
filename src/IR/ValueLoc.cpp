#include "bort/IR/ValueLoc.hpp"
#include <fmt/format.h>
#include <string>

namespace bort::ir {

auto StackLoc::toString() const -> std::string {
  return fmt::format("stack_loc .off={}", m_Offset);
}

} // namespace bort::ir
