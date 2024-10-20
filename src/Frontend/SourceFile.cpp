#include "bort/Frontend/SourceFile.hpp"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>

namespace bort {

namespace exceptions {
SourceFileReaderError::SourceFileReaderError(const std::string& message)
    : std::runtime_error{ message } {
}
SourceFileFailedToOpen::SourceFileFailedToOpen(const std::string& message)
    : SourceFileReaderError{ message } {
}
SourceFileInvalidExtension::SourceFileInvalidExtension(
    const std::string& message)
    : SourceFileReaderError{ message } {
}

} // namespace exceptions

// TODO: consider making things like this constexpr (hana or frozen)
static const std::string SmallCFileExtension{ ".c" }; // NOLINT

auto SourceFile::readSmallCFile(const SourceFileInfo& input)
    -> std::unique_ptr<SourceFile> {
  std::ifstream inputStream{ input.Path };

  if (!inputStream.good()) {
    std::string errString{ strerror(errno) };
    throw exceptions::SourceFileFailedToOpen{ fmt::format(
        "Failed to open {}\n OS message: {}", input.Path.string(),
        errString) };
  }

  if (input.Path.extension().string() != SmallCFileExtension) {
    throw exceptions::SourceFileInvalidExtension{ fmt::format(
        "Expected {} file, got {}", SmallCFileExtension,
        input.Path.extension().string()) };
  }

  std::string content{ std::istreambuf_iterator<char>{ inputStream },
                       std::istreambuf_iterator<char>{} };
  return std::unique_ptr<SourceFile>(
      new SourceFile{ input, std::move(content) });
}

} // namespace bort
