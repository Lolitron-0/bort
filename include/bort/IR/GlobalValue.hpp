#pragma once
#include "bort/Basic/Assert.hpp"
#include "bort/Basic/Casts.hpp"
#include "bort/Basic/VisitDispatcher.hpp"
#include "bort/Frontend/Symbol.hpp"
#include "bort/Frontend/Type.hpp"
#include "bort/IR/Constant.hpp"
#include "bort/IR/Value.hpp"
#include <cul/cul.hpp>
#include <fmt/format.h>
#include <unordered_map>
#include <utility>
#include <vector>

namespace bort::ir {

class GlobalValue : public Operand {
public:
  template <typename V>
  auto accept(V&& visitor);

protected:
  using Operand::Operand;

  static auto getGlobalName() -> std::string {
    static uint32_t s_Counter = 0;
    return fmt::format("_global.{}", s_Counter++);
  }
};

class GlobalVariable : public GlobalValue {
public:
  GlobalVariable(Ref<Variable> variable, ValueRef initializer)
      : GlobalValue{ variable->getType(), variable->getName() },
        m_Variable{ std::move(variable) },
        m_Initializer{ std::move(initializer) } {
  }

  [[nodiscard]] auto getSymbol() const -> Ref<Variable> {
    return m_Variable;
  }

  [[nodiscard]] auto getInitializer() const -> ValueRef {
    return m_Initializer;
  }

private:
  static std::unordered_map<Ref<Variable>, Ref<GlobalVariable>>
      m_Registry;

  Ref<Variable> m_Variable;
  ValueRef m_Initializer;
};

class GlobalInitializer : public GlobalValue {
public:
  using ElementT = Ref<IntegralConstant>;

  [[nodiscard]] auto getValues() const -> const std::vector<ElementT>& {
    return m_Values;
  }

  static auto getOrCreate(std::vector<ElementT> values,
                          std::string name = "") -> Ref<GlobalInitializer> {
    static std::unordered_map<std::string, Ref<GlobalInitializer>> m_Registry;

    name = name.empty() ? getGlobalName() : name;

    if (!m_Registry.contains(name)) {
      m_Registry[name] =
          Ref<GlobalInitializer>(new GlobalInitializer{ name, std::move(values) });
    }
    return m_Registry.at(name);
  }

private:
  GlobalInitializer(std::string name, std::vector<ElementT> values)
      : GlobalValue{ ArrayType::get(values.front()->getType(),
                                    values.size()),
                     std::move(name) },
        m_Values{ std::move(values) } {
  }

  std::vector<ElementT> m_Values;
};

template <typename V>
auto GlobalValue::accept(V&& visitor) {
  return VisitDispatcher<V, GlobalValue, GlobalInitializer,
                         GlobalVariable>::dispatch(this,
                                                   std::forward<V>(
                                                       visitor));
}

} // namespace bort::ir
