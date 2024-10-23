#include "bort/Frontend/FrontendInstance.hpp"
#include "bort/CLI/IO.hpp"
#include "bort/Frontend/SourceFile.hpp"
#include "bort/Lex/Lexer.hpp"
#include <memory>
#include <utility>

namespace bort {

FrontendFatalError::FrontendFatalError(const std::string& message)
    : std::runtime_error{ message } {
}

FrontendInstance::FrontendInstance(FrontendOptions frontendOptions)
    : m_CliOptions(std::move(frontendOptions)) {
}

void FrontendInstance::run() {
  for (auto& input : m_CliOptions.InputFiles) {
    try {
      std::shared_ptr<SourceFile> sourceFile{ SourceFile::readSmallCFile(
          input) };

      Lexer lexer;
      lexer.lex(sourceFile);
    } catch (const exceptions::SourceFileReaderError& e) {
      emitError("{}", e.what());
      DEBUG_OUT("Skipping {}", input.Path.string());
      continue;
    } catch (const FrontendFatalError& e) {
      emitError("{}", e.what());
      exit(1);
    }
  }
}

} // namespace bort
