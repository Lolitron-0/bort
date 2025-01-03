#pragma once
#include "bort/AST/ASTNode.hpp"
#include "bort/CLI/CLIOptions.hpp"
#include "bort/IR/Module.hpp"

namespace bort {
class MiddleEndInstance {
public:
  explicit MiddleEndInstance(CLIOptions cliOptions,
                             Ref<ast::ASTRoot> ast);
  auto run() -> ir::Module;

private:
  CLIOptions m_CLIOptions;
  Ref<ast::ASTRoot> m_AST;
};
} // namespace bort
