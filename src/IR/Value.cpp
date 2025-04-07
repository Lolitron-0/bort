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
          std::dynamic_pointer_cast<bort::ir::IntegralConstant>(value)) {
    return fmt::format("{}", constant->getValue());
  }
  if (bort::isaRef<Register>(value) || bort::isaRef<VariableUse>(value)) {
    return fmt::format("%{}", value->getName());
  }

  return std::nullopt;
}

Value::Value(const Value& other)
    : m_Type{ other.m_Type },
      m_Name{ other.m_Name },
      m_MDList{ makeUnique<MDList>(*other.m_MDList) } {
}

auto Value::operator=(Value other) -> Value& {
  std::swap(m_Type, other.m_Type);
  std::swap(m_Name, other.m_Name);
  std::swap(m_MDList, other.m_MDList);
  return *this;
}

Value::Value(TypeRef type, std::string name)
    : m_Type{ std::move(type) },
      m_Name{ std::move(name) },
      m_MDList{ makeUnique<MDList>() } {
}

} // namespace bort::ir
