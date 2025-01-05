#pragma once
#include "bort/Basic/Ref.hpp"
#include <memory>

namespace bort {

template <typename T1, typename T2>
auto isaRef(const Ref<T2>& obj) -> bool {
  return std::dynamic_pointer_cast<T1>(obj) != nullptr;
}

template <typename T1, typename T2>
auto dynCastRef(const Ref<T2>& obj) -> Ref<T1> {
  return std::dynamic_pointer_cast<T1>(obj);
}

} // namespace bort
