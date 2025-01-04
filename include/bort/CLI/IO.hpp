#pragma once
#include "bort/Frontend/SourceFile.hpp"
#include "bort/Lex/Token.hpp"
#include <fmt/base.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/ranges.h>

namespace bort {

class Diagnostic {
public:
  enum class Level {
    Silent = 0,
    Error,
    All
  };

  [[nodiscard]] static auto getLevel() -> Level {
    return s_Level;
  }

  static void setLevel(Level level) {
    s_Level = level;
  }

  template <typename... Args>
  static void emitError(const fmt::format_string<Args...>& message,
                        Args&&... args) {
    if (s_Level < Level::Error) {
      return;
    }
    fmt::print(stderr, fmt::fg(fmt::color::red), "error: ");
    fmt::println(stderr, message, std::forward<Args>(args)...);
  }

  template <typename... Args>
  static void emitWarning(const fmt::format_string<Args...>& message,
                          Args&&... args) {
    if (s_Level < Level::All) {
      return;
    }
    fmt::print(stderr, fmt::fg(fmt::color::magenta), "warning: ");
    fmt::println(stderr, message, std::forward<Args>(args)...);
  }

  template <typename... Args>
  static void emitError(const SourceFileIt& loc, size_t length,
                        const fmt::format_string<Args...>& message,
                        Args&&... args) {
    if (s_Level < Level::Error) {
      return;
    }
    emitError(message, std::forward<Args>(args)...);
    underlineSource(stderr, loc, length, fmt::color::red);
  }

  template <typename... Args>
  static void emitError(const Token& tok,
                        const fmt::format_string<Args...>& message,
                        Args&&... args) {
    if (s_Level < Level::Error) {
      return;
    }
    emitError(tok.getLoc(), tok.getLength(), message,
              std::forward<Args>(args)...);
  }

  template <typename... Args>
  static void emitWarning(const SourceFileIt& loc, size_t length,
                          const fmt::format_string<Args...>& message,
                          Args&&... args) {
    if (s_Level < Level::All) {
      return;
    }
    emitWarning(message, std::forward<Args>(args)...);
    underlineSource(stderr, loc, length, fmt::color::orange);
  }
  template <typename... Args>
  static void emitWarning(const Token& tok,
                          const fmt::format_string<Args...>& message,
                          Args&&... args) {
    if (s_Level < Level::All) {
      return;
    }
    emitWarning(tok.getLoc(), tok.getLength(), message,
                std::forward<Args>(args)...);
  }

  template <typename... Args>
  static void debugOut(const fmt::format_string<Args...>& message,
                       Args&&... args) {
    fmt::print(stdout, fmt::fg(fmt::color::dark_cyan), "debug: ");
    fmt::println(stdout, message, std::forward<Args>(args)...);
  }

private:
  static void underlineSource(FILE* out, const SourceFileIt& loc,
                              size_t length,
                              fmt::color color = fmt::color::gray);

  static Level s_Level;
};

#ifdef NDEBUG
#define DEBUG_OUT(...)
#define DEBUG_OUT_MSG(...)
#else
// , ## __VA_ARGS__ is a GNU extension, so just use a different name
#define DEBUG_OUT(message, ...)                                          \
  ::bort::Diagnostic::debugOut((message), __VA_ARGS__)
#define DEBUG_OUT_MSG(message, ...)                                      \
  ::bort::Diagnostic::debugOut((message))
#endif

} // namespace bort
