#pragma once
#include <filesystem>
#include <stdexcept>
#include <string>

namespace bort {

class SourceFile {
public:
  std::string m_Content;
  std::filesystem::path m_Path;
};

class SourceFileReadException : std::runtime_error {
public:
  explicit SourceFileReadException(const std::string& message);

private:
  std::string m_Message;
};

class SourceFileReader {
public:
  static auto ReadSmallCFile(const std::string& pathStr) -> SourceFile;
};

} // namespace bort
