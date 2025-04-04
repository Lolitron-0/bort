#pragma once
#include "bort/Basic/Assert.hpp"
#include "bort/Basic/Casts.hpp"
#include "bort/Basic/Ref.hpp"
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace bort {

/// if ignoreUnhandled is false, an assertion will be triggered for
/// unhandled objects
template <typename T>
struct VisitorTraits {
  static constexpr bool ignoreUnhandled = false;
  static constexpr bool isRefBased      = false;
};

/// i was utterly bored
///
/// @tparam Visitor class that has visit(...) methods for classes from
/// Hierarchy, behavior from VisitorTraits will be used for unhandled
/// elements
template <typename Visitor, typename Base, typename... Hierarchy>
class VisitDispatcher {
private:
  using VisitedTs = std::tuple<Hierarchy...>;
  template <typename T>
  using VisitDecorator =
      std::conditional_t<VisitorTraits<Visitor>::isRefBased, Ref<T>,
                         std::add_pointer_t<T>>;
  using VisitRetT              = decltype(std::declval<Visitor>().visit(
      VisitDecorator<std::tuple_element_t<0, VisitedTs>>{}));
  static constexpr bool isVoid = std::is_void_v<VisitRetT>;

  static_assert((std::is_same_v<decltype(std::declval<Visitor>().visit(
                                    VisitDecorator<Hierarchy>{})),
                                VisitRetT> &&
                 ...),
                "All Derived::visit return types must be the same");

public:
  template <typename RawTo>
  static auto getCasted(const VisitDecorator<Base>& value)
      -> VisitDecorator<RawTo> {
    if constexpr (VisitorTraits<Visitor>::isRefBased) {
      return dynCastRef<RawTo>(value);
    } else {
      return dynamic_cast<RawTo*>(value);
    }
  }

  template <typename V>
    requires std::is_same_v<std::remove_cvref_t<V>,
                            std::remove_cvref_t<Visitor>>
  static auto dispatch(VisitDecorator<Base> value,
                       V&& visitor) -> VisitRetT {
    return [&]<size_t... Is>(std::index_sequence<Is...>) {
      using VisitRetSafeT = std::conditional_t<isVoid, void*, VisitRetT>;
      std::optional<VisitRetSafeT> res;
      bool found = false;
      (
          [&]<size_t i> {
            auto v{ getCasted<std::tuple_element_t<i, VisitedTs>>(
                value) };
            if (v) {
              found = true;
              if constexpr (isVoid) {
                std::forward<Visitor>(visitor).visit(v);
              } else {
                res = std::forward<Visitor>(visitor).visit(v);
              }
            }
          }.template operator()<Is>(),
          ...);
      if constexpr (!VisitorTraits<Visitor>::ignoreUnhandled) {
        bort_assert(found, "Object type not handled");
      }
      if constexpr (!isVoid) {
        return *res;
      }
    }.template operator()(std::index_sequence_for<Hierarchy...>());
  }
};

} // namespace bort
