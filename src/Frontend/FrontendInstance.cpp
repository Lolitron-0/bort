#include "bort/Frontend/FrontendInstance.hpp"
#include "bort/CLI/IO.hpp"
#include "bort/Frontend/InputFile.hpp"
#include <utility>

namespace bort {

FrontendInstance::FrontendInstance(FrontendOptions frontendOptions)
    : m_FrontendOptions(std::move(frontendOptions)) {
}

void FrontendInstance::Run() {
  for (auto& input : m_FrontendOptions.InputFiles) {
    try {
      auto inputContent{ SourceFileReader::ReadSmallCFile(input) };
    } catch (const exceptions::SourceFileReaderError& e) {
      EmitError(e.what());
      DEBUG_OUT(std::string{ "Skipping " } + input.Path.string());
      continue;
    }
  }
}

} // namespace bort
