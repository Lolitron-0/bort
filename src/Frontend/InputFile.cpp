#include "bort/Frontend/InputFile.hpp"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>

namespace bort {

SourceFileReadException::SourceFileReadException(
    const std::string& message)
    : std::runtime_error{ "SourceFileReader: " + message } {
}

// TODO: consider making things like this constexpr (hana or frozen)
static const std::string SmallCFileExtension{ "c" }; // NOLINT

auto SourceFileReader::ReadSmallCFile(const InputFile& input)
    -> std::string {
  std::ifstream inputStream{ input.Path };

  if (!inputStream.good()) {
    std::string errString{ strerror(errno) };
    throw SourceFileReadException{ "Failed to open " +
                                   input.Path.string() +
                                   "\nOS message: " + errString };
  }

  if (input.Path.extension().string() != SmallCFileExtension) {
    throw SourceFileReadException{ "Expected ." + SmallCFileExtension +
                                   " file" };
  }

  std::string content{ std::istreambuf_iterator<char>{ inputStream },
                       std::istreambuf_iterator<char>{} };
  return content;
}

} // namespace bort
