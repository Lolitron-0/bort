#include "bort/CLI/IO.hpp"
#include "bort/Frontend/FrontendInstance.hpp"
#include "bort/Frontend/FrontendOptions.hpp"
#include <algorithm>
#include <cxxopts.hpp>
#include <iostream>

auto main(int argc, char* argv[]) -> int {

  bort::FrontendOptions frontendOptions;

  cxxopts::Options cliOptions{ "bort", "Small-C to RISC-V compiler" };
  // clang-format off
  cliOptions.add_options()
      ("h,help", "Show help")
      ("inputs", "Small-C files to compile", cxxopts::value<std::vector<std::string>>());
  // clang-format on

  cliOptions.parse_positional("inputs");

  auto result{ cliOptions.parse(argc, argv) };

  if (result.count("help")) {
    std::cout << cliOptions.help() << std::endl;
    return 0;
  }

  if (result.count("inputs") == 0) {
    bort::emitError("No input files");
    return 1;
  }

  auto inputs{ result["inputs"].as<std::vector<std::string>>() };

  std::transform(inputs.begin(), inputs.end(),
                 std::back_inserter(frontendOptions.InputFiles),
                 [](auto&& input) {
                   return bort::SourceFileInfo{ .Path = input };
                 });

  bort::FrontendInstance frontend{ std::move(frontendOptions) };
  frontend.run();

  return 0;
}
