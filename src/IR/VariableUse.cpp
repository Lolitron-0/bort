#include "bort/IR/VariableUse.hpp"
#include <algorithm>

namespace bort::ir {

std::unordered_map<Ref<Variable>, Ref<VariableUse>> VariableUse::s_Uses{};

auto VariableUse::createUnique(const Ref<Variable>& variable)
    -> ValueRef {
  bort_assert(!s_Uses.contains(variable), "VariableUse already created");
  auto name{ variable->getName() };
  auto existingIt = std::ranges::find_if(s_Uses, [&name](auto pair) {
    return pair.first->getName() == name;
  });
  if (existingIt != s_Uses.end()) {
    name = fmt::format("{}_{}", existingIt->second->getName(),
                       existingIt->second->m_NameCounter++);
  }
  s_Uses[variable] =
      Ref<VariableUse>{ new VariableUse{ variable, std::move(name) } };
  return s_Uses.at(variable);
}

auto VariableUse::get(const Ref<Variable>& variable) -> ValueRef {
  bort_assert(s_Uses.contains(variable), "VariableUse not found");
  return s_Uses.at(variable);
}

} // namespace bort::ir
