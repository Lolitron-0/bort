#include "bort/Codegen/BackendInstance.hpp"
#include "bort/Codegen/RISCVCodegen.hpp"
#include "bort/Codegen/RISCVPrinter.hpp"
#include "bort/IR/IRPrinter.hpp"
#include <utility>

using namespace bort;

BackendInstance::BackendInstance(CLIOptions cliOptions,
                                 ir::Module module)
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

  codegen::rv::Printer riscvPrinter{ std::cerr };
  riscvPrinter.run(m_Module);
}
