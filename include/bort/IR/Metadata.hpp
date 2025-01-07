#pragma once
#include "bort/Basic/Casts.hpp"
#include "bort/Basic/Ref.hpp"
#include <ranges>
#include <string>
#include <typeindex>
#include <unordered_map>

namespace bort::ir {

struct Metadata {
  virtual ~Metadata()                                        = default;
  [[nodiscard]] virtual auto toString() const -> std::string = 0;
};

template <typename T>
concept MetadataClass =
    std::is_base_of_v<Metadata, T> && std::is_copy_constructible_v<T>;

class MDList {
public:
  MDList() = default;

  template <MetadataClass T>
  void add(T data) {
    m_Registry.insert({ typeid(T), makeRef<T>(std::move(data)) });
  }

  template <MetadataClass T>
  auto contains() const -> bool {
    return m_Registry.contains(typeid(T));
  }

  template <MetadataClass T>
  auto get() -> T* {
    auto node{ contains<T>()
                   ? dynCastRef<T>(m_Registry.at(typeid(T))).get()
                   : nullptr };
    return node;
  }

  auto nodes() {
    return std::views::values(m_Registry);
  }

private:
  std::unordered_map<std::type_index, Ref<Metadata>> m_Registry;
};

} // namespace bort::ir
