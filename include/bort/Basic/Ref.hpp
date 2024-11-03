#pragma once
#include <memory>

namespace bort {

// in case i'll need to make my own ref counting
// (and to write less)

template <typename T>
using Ref = std::shared_ptr<T>;

template <typename T>
using Unique = std::unique_ptr<T>;

template <typename T, typename... Args>
auto makeRef(Args&&... args) -> Ref<T> {
  return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
auto makeUnique(Args&&... args) -> Unique<T> {
  return std::make_unique<T>(std::forward<Args>(args)...);
}

} // namespace bort
