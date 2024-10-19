#include "bort/Frontend/InputFile.hpp"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>

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
static const std::string SmallCFileExtension{ "c" }; // NOLINT

auto SourceFileReader::ReadSmallCFile(const InputFile& input)
    -> InputFileBuffer {
  std::ifstream inputStream{ input.Path };

  if (!inputStream.good()) {
    std::string errString{ strerror(errno) };
    throw exceptions::SourceFileFailedToOpen{
      "Failed to open " + input.Path.string() +
      "\nOS message: " + errString
    };
  }

  if (input.Path.extension().string() != SmallCFileExtension) {
    throw exceptions::SourceFileInvalidExtension{
      "Expected ." + SmallCFileExtension + " file"
    };
  }

  std::string content{ std::istreambuf_iterator<char>{ inputStream },
                       std::istreambuf_iterator<char>{} };
  return content;
}

} // namespace bort
