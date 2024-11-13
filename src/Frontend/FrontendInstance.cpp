#include "bort/Frontend/FrontendInstance.hpp"
#include "bort/AST/Visitors/ASTPrinter.hpp"
#include "bort/AST/Visitors/SymbolResolutionVisitor.hpp"
#include "bort/AST/Visitors/TypePropagationVisitor.hpp"
#include "bort/Basic/Assert.hpp"
#include "bort/CLI/IO.hpp"
#include "bort/Frontend/SourceFile.hpp"
#include "bort/Lex/Lexer.hpp"
#include "bort/Parse/Parser.hpp"
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

      /// @todo preprocessing
      if (m_CliOptions.PreprocessorOnly) {
        emitError("Preprocessing is not yet implemented");
        continue;
      }

      Lexer lexer;
      lexer.lex(sourceFile);
      Parser parser{ lexer.getTokens() };
      auto ast{ parser.buildAST() };

      if (parser.isASTInvalid()) {
        continue;
      }

      ast::SymbolResolutionVisitor symbolResolveVisitor{};
      symbolResolveVisitor.SAVisit(ast);
      if (symbolResolveVisitor.isASTInvalidated()) {
        DEBUG_OUT_MSG("Symbol resolution pass failed. Aborting");
        continue;
      }

      ast::TypePropagationVisitor typePropagationVisitor{};
      typePropagationVisitor.SAVisit(ast);
      if (typePropagationVisitor.isASTInvalidated()) {
        DEBUG_OUT_MSG("Type propagation pass failed. Aborting");
        continue;
      }

      if (m_CliOptions.DumpAST) {
        ast::ASTPrinter astPrinter{};
        astPrinter.SAVisit(ast);
      }

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
