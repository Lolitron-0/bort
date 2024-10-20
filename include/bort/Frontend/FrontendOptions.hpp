#pragma once
#include "bort/Frontend/SourceFile.hpp"
#include <vector>

namespace bort {

class FrontendOptions {
public:
  std::vector<SourceFileInfo> InputFiles;

  // TODO: glue all inputs to single output
};

} // namespace bort
