#pragma once
#include <boost/assert.hpp>
#include <boost/stacktrace.hpp>
#include <fmt/color.h>
#include <fmt/core.h>
#include <iostream>

#define bort_assert_nomsg(expr) BOOST_ASSERT((expr))
#define bort_assert(expr, msg) BOOST_ASSERT_MSG((expr), (msg))

namespace boost {
inline void assertion_failed_msg(char const* expr, char const* msg,
                                 char const* function,
                                 char const* /*file*/, long /*line*/) {
  fmt::print(stderr, fmt::fg(fmt::color::red), "Assertion failed: ");
  fmt::print(stderr, "Expression '{}' is false in function '{}': {}.\n",
             expr, function, (msg ? msg : "<...>"));
  fmt::print(stderr, fmt::fg(fmt::color::orange), "Stacktrace:\n");
  std::cerr << boost::stacktrace::stacktrace() << std::endl;
  std::abort();
}

inline void assertion_failed(char const* expr, char const* function,
                             char const* file, long line) {
  ::boost::assertion_failed_msg(expr, nullptr, function, file, line);
}
} // namespace boost
