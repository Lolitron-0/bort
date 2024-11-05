#pragma once
#include "bort/Frontend/SourceFile.hpp"
#include <vector>

namespace bort {

/// @todo Glue all inputs to single output
class FrontendOptions {
public:
  std::vector<SourceFileInfo> InputFiles;

  bool PreprocessorOnly{ false };
  bool DumpAST{ false };
};

} // namespace bort