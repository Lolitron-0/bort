#include "bort/Frontend/FrontEndInstance.hpp"
#include "bort/AST/Visitors/ASTPrinter.hpp"
#include "bort/AST/Visitors/ASTVisitor.hpp"
#include "bort/AST/Visitors/SymbolResolutionVisitor.hpp"
#include "bort/AST/Visitors/TypePropagationVisitor.hpp"
#include "bort/CLI/IO.hpp"
#include "bort/Frontend/SourceFile.hpp"
#include "bort/Lex/Lexer.hpp"
#include "bort/Parse/Parser.hpp"
#include <memory>
#include <utility>

namespace bort {

FrontEndFatalError::FrontEndFatalError(const std::string& message)
    : std::runtime_error{ message } {
}

FrontEndInstance::FrontEndInstance(CLIOptions cliOptions)
    : m_CLIOptions(std::move(cliOptions)) {
}

auto FrontEndInstance::run() -> Ref<ast::ASTRoot> {
  for (auto& input : m_CLIOptions.InputFiles) {
    try {
      std::shared_ptr<SourceFile> sourceFile{ SourceFile::readSmallCFile(
          input) };

      /// @todo preprocessing
      if (m_CLIOptions.PreprocessorOnly) {
        Diagnostic::emitError("Preprocessing is not yet implemented");
        continue;
      }

      Lexer lexer;
      lexer.lex(sourceFile);
      Parser parser{ lexer.getTokens() };
      auto ast{ parser.buildAST() };

      if (parser.isASTInvalid()) {
        throw FrontEndFatalError{ fmt::format(
            "Fatal error parsing {}, build stopped",
            input.Path.string()) };
      }

      ast::SymbolResolutionVisitor symbolResolveVisitor{};
      symbolResolveVisitor.SAVisit(ast);
      if (symbolResolveVisitor.isASTInvalidated()) {
        DEBUG_OUT_MSG("Symbol resolution pass failed. Skipping file");
        continue;
      }

      ast::TypePropagationVisitor typePropagationVisitor{};
      typePropagationVisitor.SAVisit(ast);
      if (typePropagationVisitor.isASTInvalidated()) {
        DEBUG_OUT_MSG("Type propagation pass failed. Skipping file");
        continue;
      }

      if (m_CLIOptions.DumpAST) {
        ast::ASTPrinter astPrinter{};
        astPrinter.SAVisit(ast);
      }

      return ast;

    } catch (const exceptions::SourceFileReaderError& e) {
      Diagnostic::emitError("{}", e.what());
      DEBUG_OUT("Skipping {}", input.Path.string());
      continue;
    } catch (const FrontEndFatalError& e) {
      Diagnostic::emitError("{}", e.what());
      std::exit(1);
    }
  }

  return nullptr;
}

} // namespace bort
