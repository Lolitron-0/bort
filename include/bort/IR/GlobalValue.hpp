#pragma once
#include "bort/Basic/Assert.hpp"
#include "bort/Basic/Casts.hpp"
#include "bort/Basic/VisitDispatcher.hpp"
#include "bort/Frontend/Type.hpp"
#include "bort/IR/Constant.hpp"
#include "bort/IR/Value.hpp"
#include <cul/cul.hpp>
#include <fmt/format.h>
#include <unordered_map>
#include <utility>
#include <vector>

namespace bort::ir {

class GlobalValue : public Value {
public:
  template <typename V>
  auto accept(V&& visitor);

protected:
  using Value::Value;

  static auto getGlobalName() -> std::string {
    static uint32_t s_Counter = 0;
    return fmt::format("_global.{}", s_Counter++);
  }
};

class GlobalArray : public GlobalValue {
public:
  using ElementT = Ref<IntegralConstant>;

  [[nodiscard]] auto getValues() const -> const std::vector<ElementT>& {
    return m_Values;
  }

  static auto getOrCreate(std::vector<ElementT> values,
                          std::string name = "") -> Ref<GlobalArray> {
    static std::unordered_map<std::string, Ref<GlobalArray>> m_Registry;

    name = name.empty() ? getGlobalName() : name;

    if (!m_Registry.contains(name)) {
      m_Registry[name] =
          Ref<GlobalArray>(new GlobalArray{ name, std::move(values) });
    }
    return m_Registry.at(name);
  }

private:
  GlobalArray(std::string name, std::vector<ElementT> values)
      : GlobalValue{ ArrayType::get(values.front()->getType(),
                                    values.size()),
                     std::move(name) },
        m_Values{ std::move(values) } {
  }

  std::vector<ElementT> m_Values;
};

template <typename V>
auto GlobalValue::accept(V&& visitor) {
  return VisitDispatcher<V, GlobalValue, GlobalArray>::dispatch(
      this, std::forward<V>(visitor));
}

} // namespace bort::ir
