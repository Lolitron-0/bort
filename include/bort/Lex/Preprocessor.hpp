#pragma once
#include "bort/Frontend/InputFile.hpp"
#include <string>
#include <unordered_map>

namespace bort {

class PreprocessorInstance {
public:

private:
  std::unordered_map<std::string, std::string> m_MacroDefinitions;
};

} // namespace bort
