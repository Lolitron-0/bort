#include "bort/CLI/CLIOptions.hpp"
#include "bort/CLI/IO.hpp"
#include "bort/Codegen/BackendInstance.hpp"
#include "bort/Frontend/FrontEndInstance.hpp"
#include "bort/IR/MiddleEndInstance.hpp"
#include <algorithm>
#include <cxxopts.hpp>
#include <iostream>
#ifdef WIN32
#include <windows.h>
#endif

auto main(int argc, char* argv[]) -> int {

/// @todo: Make custom wrappers for fmt color formatters to make color
/// optional
#ifdef WIN32
  HANDLE hOut{ GetStdHandle(STD_OUTPUT_HANDLE) };
  HANDLE hErr{ GetStdHandle(STD_ERROR_HANDLE) };
  DWORD dwMode{};

  GetConsoleMode(hOut, &dwMode);
  dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  SetConsoleMode(hOut, dwMode);

  GetConsoleMode(hErr, &dwMode);
  dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  SetConsoleMode(hErr, dwMode);
#endif

  bort::CLIOptions cliOptions;

  cxxopts::Options cliParser{ "bort", "Small-C to RISC-V compiler" };
  // clang-format off
  cliParser.add_options()
      ("h,help", "Show help")
      // ("E,preprocess", "Just preprocess file and dump it to stdout",
      //  cxxopts::value<bool>(cliOptions.PreprocessorOnly))
      ("dump-ast", "Dump AST to stderr",
       cxxopts::value<bool>(cliOptions.DumpAST))
      ("emit-ir", "Dump IR to stderr",
       cxxopts::value<bool>(cliOptions.EmitIR))
      ("dump-codegen-info", "Dump IR after codegen to stderr",
       cxxopts::value<bool>(cliOptions.DumpCodegenInfo))
      ("o", "Output file, '-' for stdout",
       cxxopts::value<std::string>(cliOptions.OutputFilename))
      ("inputs", "Small-C files to compile", cxxopts::value<std::vector<std::string>>());
  // clang-format on

  cliParser.parse_positional("inputs");
  cliParser.positional_help("inputs");
  cliParser.show_positional_help();

  auto result{ cliParser.parse(argc, argv) };

  if (result.count("help")) {
    std::cout << cliParser.help() << std::endl;
    return 0;
  }

  if (result.count("inputs") == 0) {
    bort::Diagnostic::emitError("No input files");
    return 1;
  }

  if (result.count("o") == 0) {
    bort::Diagnostic::emitError(
        "-o <output-destination> flag is required");
    return 1;
  }

  auto inputs{ result["inputs"].as<std::vector<std::string>>() };

  std::transform(inputs.begin(), inputs.end(),
                 std::back_inserter(cliOptions.InputFiles),
                 [](auto&& input) {
                   return bort::SourceFileInfo{ .Path = input };
                 });

  bort::FrontEndInstance frontend{ cliOptions };
  auto ast{ frontend.run() };
  if (!ast) {
    std::exit(1);
  }

  bort::MiddleEndInstance middleEnd{ cliOptions, std::move(ast) };
  auto IR{ middleEnd.run() };

  bort::BackendInstance backend{ cliOptions, std::move(IR) };
  backend.run();

  return 0;
}
