#pragma once
#include "bort/Frontend/SourceFile.hpp"
#include <vector>

namespace bort {

/// @todo Glue all inputs to single output
class CLIOptions {
public:
  std::vector<SourceFileInfo> InputFiles;

  bool PreprocessorOnly{ false };
  bool DumpAST{ false };
  bool EmitIR{ false };
  bool DumpCodegenInfo{false};
};

} // namespace bort
