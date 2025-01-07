#include "bort/IR/Value.hpp"
#include "bort/IR/Constant.hpp"
#include "bort/IR/Register.hpp"
#include "bort/IR/VariableUse.hpp"
#include <fmt/format.h>
#include <optional>

namespace bort::ir {

auto Value::formatValue(const ValueRef& value)
    -> std::optional<std::string> {
  if (auto constant =
          std::dynamic_pointer_cast<bort::ir::IntConstant>(value)) {
    return fmt::format("{}", constant->getValue());
  }
  if (bort::isaRef<Register>(value) || bort::isaRef<VariableUse>(value)) {
    return fmt::format("%{}", value->getName());
  }

  return std::nullopt;
}

} // namespace bort::ir
