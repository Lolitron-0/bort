#include "bort/IR/MiddleEndInstance.hpp"
#include "bort/Codegen/RISCVCodegen.hpp"
#include "bort/IR/IRCodegen.hpp"
#include "bort/IR/IRPrinter.hpp"
#include "bort/IR/Module.hpp"

namespace bort {

MiddleEndInstance::MiddleEndInstance(CLIOptions cliOptions,
                                     Ref<ast::ASTRoot> ast)
    : m_CLIOptions(std::move(cliOptions)),
      m_AST(std::move(ast)) {
}

auto MiddleEndInstance::run() -> ir::Module {
  ir::IRCodegen irCodegen{};
  irCodegen.codegen(m_AST);
  auto IR{ irCodegen.takeInstructions() };

  codegen::rv::Generator riscvCodegen{ m_CLIOptions, IR };
  riscvCodegen.generate();

  if (m_CLIOptions.EmitIR) {
    ir::IRPrinter irPrinter{};
    irPrinter.print(IR);
  }
  return IR;
}

} // namespace bort
