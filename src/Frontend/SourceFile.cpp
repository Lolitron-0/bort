#include "bort/Frontend/SourceFile.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>

namespace bort {

SourceFileReadException::SourceFileReadException(
    const std::string& message)
    : std::runtime_error{ "SourceFileReader: " + message } {
}

constexpr std::string s_SmallCFileExtension{ "c" };

auto SourceFileReader::ReadSmallCFile(const std::string& pathStr)
    -> SourceFile {
  std::ifstream stream{ pathStr };

  if (!stream.good()) {
  }

  std::filesystem::path path{ pathStr };

  std::stringstream sstream;
  sstream << stream.rdbuf();
}

} // namespace bort
