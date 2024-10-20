#include "bort/Frontend/FrontendInstance.hpp"
#include "bort/CLI/IO.hpp"
#include "bort/Frontend/SourceFile.hpp"
#include "bort/Lex/Lexer.hpp"
#include <memory>
#include <utility>

namespace bort {

FrontendInstance::FrontendInstance(FrontendOptions frontendOptions)
    : m_FrontendOptions(std::move(frontendOptions)) {
}

void FrontendInstance::run() {
  for (auto& input : m_FrontendOptions.InputFiles) {
    try {
      std::shared_ptr<SourceFile> sourceFile{ SourceFile::readSmallCFile(
          input) };
      Lexer lexer;
      lexer.lex(sourceFile);
    } catch (const exceptions::SourceFileReaderError& e) {
      emitError("{}", e.what());
      DEBUG_OUT("Skipping {}", input.Path.string());
      continue;
    }
  }
}

} // namespace bort
