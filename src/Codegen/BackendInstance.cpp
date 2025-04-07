#include "bort/Codegen/BackendInstance.hpp"
#include "bort/Basic/Ref.hpp"
#include "bort/Codegen/RISCVCodegen.hpp"
#include "bort/Codegen/RISCVPrinter.hpp"
#include "bort/IR/IRPrinter.hpp"
#include <fmt/format.h>
#include <fstream>
#include <utility>

using namespace bort;

BackendInstance::BackendInstance(CLIOptions cliOptions, ir::Module module)
    : m_CLIOptions{ std::move(cliOptions) },
      m_Module{ std::move(module) } {
}

void bort::BackendInstance::run() {
  codegen::rv::Generator riscvCodegen{ m_CLIOptions, m_Module };
  riscvCodegen.generate();

  if (m_CLIOptions.DumpCodegenInfo) {
    std::cerr << "\n=== After codegen ===\n\n";
    ir::IRPrinter irPrinter{};
    irPrinter.print(m_Module);
  }

  Unique<codegen::rv::Printer> riscvPrinter;
  Unique<std::ofstream> outStream;
  if (m_CLIOptions.OutputFilename == "-") {
    riscvPrinter = makeUnique<codegen::rv::Printer>(std::cout);
  } else {
    outStream = makeUnique<std::ofstream>(m_CLIOptions.OutputFilename);

    if (!outStream->good()) {
      std::string errString{ strerror(errno) };
      throw BackendFatalError{ fmt::format(
          "Failed to open output file: {}\nOS message: {}",
          m_CLIOptions.OutputFilename, std::move(errString)) };
    }

    riscvPrinter = makeUnique<codegen::rv::Printer>(*outStream);
  }

  riscvPrinter->run(m_Module);
}
