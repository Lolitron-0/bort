#pragma once
#include <filesystem>
#include <stdexcept>
#include <string>

namespace bort {

class InputFile {
public:
  std::filesystem::path Path;
};

class SourceFileReadException : std::runtime_error {
public:
  explicit SourceFileReadException(const std::string& message);

private:
  std::string m_Message;
};

class SourceFileReader {
public:
  static auto ReadSmallCFile(const InputFile& input) -> std::string;
};

} // namespace bort
