#pragma once
#include <filesystem>
#include <fmt/format.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

namespace bort {

using SourceFileBuffer = std::string;

struct SourceFileInfo {
public:
  std::filesystem::path Path;
};

class SourceFile {
public:
  [[nodiscard]] auto getInfo() const -> SourceFileInfo {
    return m_Info;
  }

  [[nodiscard]] auto getBuffer() const -> const SourceFileBuffer& {
    return m_Buffer;
  }

  [[nodiscard]] auto getBuffer() -> SourceFileBuffer& {
    return m_Buffer;
  }

  static auto readSmallCFile(const SourceFileInfo& input)
      -> std::unique_ptr<SourceFile>;

private:
  // SourceFileInfo wraps it's buffer entirely, so to be safe, just moving
  // it here
  SourceFile(SourceFileInfo info, SourceFileBuffer&& buffer)
      : m_Info{ std::move(info) },
        m_Buffer{ std::move(buffer) } {
  }
  SourceFileInfo m_Info;
  SourceFileBuffer m_Buffer;
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

// This is read-only wrapper around a SourceFileBuffer,
// which provides some additional info.
// It is expected that this is used as a scanline through file buffer, so
// no creation from exact position is implemented (that's why some getters
// return iterators of the underlying buffer)
class SourceFileIt {
public:
  using difference_type = SourceFileBuffer::difference_type;

  explicit SourceFileIt(const std::shared_ptr<SourceFile>& file)
      : m_File{ file },
        m_LineEndIdx{ file->getBuffer().find('\n') } {
    if (m_LineEndIdx == std::string::npos) {
      m_LineEndIdx = file->getBuffer().size() - 1;
    }
  }

  auto operator++() -> SourceFileIt& {
    if (m_File->getBuffer().at(m_Index) == '\n') {
      m_Line++;
      m_Column       = 1;
      m_LineStartIdx = m_Index + 1;
      m_LineEndIdx   = m_File->getBuffer().find('\n', m_LineStartIdx);
      if (m_LineEndIdx == std::string::npos) {
        m_LineEndIdx = m_File->getBuffer().size() - 1;
      }
    }
    m_Column++;
    m_Index++;
    return *this;
  }

  auto operator++(int) -> SourceFileIt {
    auto it{ *this };
    operator++();
    return it;
  }

  [[nodiscard]] auto getIndex() const -> size_t {
    return m_Index;
  }
  [[nodiscard]] auto getLineNum() const -> size_t {
    return m_Line;
  }
  [[nodiscard]] auto getColumnNum() const -> size_t {
    return m_Column;
  }
  [[nodiscard]] auto getCurrentLineLength() const -> size_t {
    return m_LineEndIdx - m_LineStartIdx;
  }
  [[nodiscard]] auto getLineStartBufIt() const
      -> SourceFileBuffer::const_iterator {
    return m_File->getBuffer().begin() +
           static_cast<difference_type>(m_LineStartIdx);
  }
  [[nodiscard]] auto getLineEndBufIt() const
      -> SourceFileBuffer::const_iterator {
    return m_File->getBuffer().begin() +
           static_cast<difference_type>(m_LineEndIdx);
  }
  [[nodiscard]] auto getCurrentLine() const -> std::string_view {
    return { getLineStartBufIt(), getLineEndBufIt() };
  }

    [[nodiscard]] auto getValue(size_t length) const -> std::string_view {
    return std::string_view{
      asBufIter(),
      asBufIter() + static_cast<SourceFileIt::difference_type>(length)
    };
  };

  auto operator*() const -> const char& {
    return m_File->getBuffer().at(m_Index);
  }

  auto operator+=(size_t n) -> SourceFileIt& {
    for (int i{ 0 }; i < static_cast<int>(n); ++i) {
      operator++();
    }
    return *this;
  }

  [[nodiscard]] auto asBufIter() const
      -> SourceFileBuffer::const_iterator {
    return static_cast<SourceFileBuffer::const_iterator>(*this);
  }

  [[nodiscard]] auto toString() const -> std::string {
    return std::string{ *this };
  }

  explicit operator std::string() const {
    return fmt::format("{}:{}:{}", m_File->getInfo().Path.string(),
                       m_Line, m_Column);
  }

  explicit operator bool() const {
    return m_Index < m_File->getBuffer().size();
  }

  constexpr /* implicit */ operator SourceFileBuffer::const_iterator()
      const {
    return m_File->getBuffer().begin() +
           static_cast<difference_type>(m_Index);
  }

private:
  std::shared_ptr<SourceFile> m_File;
  size_t m_LineStartIdx{ 0 };
  size_t m_LineEndIdx;
  size_t m_Index{ 0 };
  size_t m_Line{ 1 };
  size_t m_Column{ 1 };
};

} // namespace bort
