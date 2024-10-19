#pragma once
#include "bort/Frontend/InputFile.hpp"
#include <cstddef>

namespace bort {

struct SourceLocation {
public:
  SourceLocation(const InputFile& file, size_t line, size_t column,
                 InputFileBufferPos pos)
      : File{ &file },
        Line{ line },
        Column{ column },
        Position{ pos } {
  }

  const InputFile* File;
  size_t Line;
  size_t Column;
  InputFileBufferPos Position;
};

} // namespace bort
