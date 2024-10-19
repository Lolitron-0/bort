#pragma once
#include <filesystem>
#include <stdexcept>
#include <string>

namespace bort {

class InputFile {
public:
  std::filesystem::path Path;
};

namespace exceptions {

class SourceFileReaderError : public std::runtime_error {
public:
  explicit SourceFileReaderError(const std::string& message);
};

class SourceFileFailedToOpen : public SourceFileReaderError {
public:
  explicit SourceFileFailedToOpen(const std::string& message);

private:
  std::string m_Message;
};

class SourceFileInvalidExtension : public SourceFileReaderError {
public:
  explicit SourceFileInvalidExtension(const std::string& message);

private:
  std::string m_Message;
};

} // namespace exceptions

using InputFileBuffer = std::string;
using InputFileBufferPos = InputFileBuffer::iterator;

class SourceFileReader {
public:
  static auto ReadSmallCFile(const InputFile& input) -> InputFileBuffer;
};

} // namespace bort
