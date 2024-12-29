#include "bort/IR/Value.hpp"
#include "bort/Basic/Assert.hpp"
#include <fmt/format.h>

namespace bort::ir {

std::unordered_map<std::string, ValueRef> Value::s_Values{};

auto Value::createUnique(TypeRef type, std::string name) -> ValueRef {
  if (s_Values.contains(name)) {
    auto& existingValue{ s_Values.at(name) };
    name = fmt::format("{}_{}", existingValue->getName(),
                       existingValue->m_NameCounter++);
  }
  s_Values[name] = ValueRef(new Value{ std::move(type), name });
  return s_Values.at(name);
};

auto Value::get(std::string name) -> ValueRef {
  bort_assert(s_Values.contains(name),
              fmt::format("Value {} not found", name).c_str());
  return s_Values.at(name);
}

} // namespace bort::ir
