#pragma once
#include "bort/Frontend/SourceFile.hpp"
#include <memory>
#include <unordered_map>

namespace bort {

class Preprocessor {
public:
  void preprocess(const std::shared_ptr<SourceFile>& file);

private:
  void defineIdentifier(SourceFileIt& pos);
  void undefIdentifier(std::string_view ident);

  std::unordered_map<std::string, std::string>
      m_MacroDefinitions;
};

} // namespace bort
