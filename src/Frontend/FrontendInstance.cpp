#include "bort/Frontend/FrontendInstance.hpp"
#include "bort/CLI/IO.hpp"
#include "bort/Frontend/SourceFile.hpp"
#include "bort/Lex/Lexer.hpp"
#include "bort/Lex/Preprocessor.hpp"
#include <iostream>
#include <memory>
#include <utility>

namespace bort {

FrontendInstance::FrontendInstance(FrontendOptions frontendOptions)
    : m_CliOptions(std::move(frontendOptions)) {
}

void FrontendInstance::run() {
  for (auto& input : m_CliOptions.InputFiles) {
    try {
      std::shared_ptr<SourceFile> sourceFile{ SourceFile::readSmallCFile(
          input) };
      // Preprocessor pp;
      // pp.preprocess(sourceFile);

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
