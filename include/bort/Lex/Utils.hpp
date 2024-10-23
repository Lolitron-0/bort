#pragma once
#include "bort/Frontend/SourceFile.hpp"
#include <algorithm>
#include <array>
#include <string_view>

namespace bort {

auto startsWith(const SourceFileIt& pos, std::string_view prefix) -> bool;

void skipSpacesSince(SourceFileIt& pos);

auto consumeIdent(SourceFileIt& pos) -> std::string_view;

template <typename T, size_t N>
constexpr auto sortConstexpr(std::array<T, N> arr,
                             bool (*less)(const std::decay_t<T>&,
                                          const std::decay_t<T>&))
    -> std::array<T, N> {
  for (std::size_t i{ 0 }; i < N - 1; ++i) {
    for (std::size_t j{ i + 1 }; j < N; ++j) {
      if (less(arr[j], arr[i])) {
        std::swap(arr[i], arr[j]);
      }
    }
  }
  return arr;
}

} // namespace bort
