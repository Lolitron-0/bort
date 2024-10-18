#pragma once
#include "bort/Frontend/InputFile.hpp"
#include <vector>

namespace bort {

class FrontendOptions {
public:
  std::vector<InputFile> InputFiles;

  // TODO: glue all inputs to single output
};

} // namespace bort
