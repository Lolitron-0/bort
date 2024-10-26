#pragma once
#include "bort/Frontend/SourceFile.hpp"
#include "bort/Lex/Token.hpp"
#include <fmt/base.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <string_view>

namespace bort {

template <typename... Args>
void emitError(const fmt::format_string<Args...>& message,
               Args&&... args) {
  fmt::print(stderr, fmt::fg(fmt::color::red), "error: ");
  fmt::println(stderr, message, std::forward<Args>(args)...);
}

void underlineSource(FILE* out, const SourceFileIt& loc, size_t length,
                     fmt::color color = fmt::color::gray);

template <typename... Args>
void emitWarning(const fmt::format_string<Args...>& message,
                 Args&&... args) {

  fmt::print(stderr, fmt::fg(fmt::color::magenta), "warning: ");
  fmt::println(stderr, message, std::forward<Args>(args)...);
}

template <typename... Args>
void emitError(const SourceFileIt& loc, size_t length,
               const fmt::format_string<Args...>& message,
               Args&&... args) {
  emitError(message, std::forward<Args>(args)...);
  underlineSource(stderr, loc, length, fmt::color::red);
}

template <typename... Args>
void emitError(const Token& tok,
               const fmt::format_string<Args...>& message,
               Args&&... args) {
  emitError(tok.getLoc(), tok.getLength(), message,
            std::forward<Args>(args)...);
}

template <typename... Args>
void emitWarning(const SourceFileIt& loc, size_t length,
                 const fmt::format_string<Args...>& message,
                 Args&&... args) {
  emitWarning(message, std::forward<Args>(args)...);
  underlineSource(stderr, loc, length, fmt::color::orange);
}

template <typename... Args>
void debugOut(const fmt::format_string<Args...>& message,
              Args&&... args) {
  fmt::print(stdout, fmt::fg(fmt::color::dark_cyan), "debug: ");
  fmt::println(stdout, message, std::forward<Args>(args)...);
}

#ifdef NDEBUG
#define DEBUG_OUT(...)
#else
#define DEBUG_OUT(message, ...) ::bort::debugOut((message), __VA_ARGS__)
#endif

} // namespace bort
