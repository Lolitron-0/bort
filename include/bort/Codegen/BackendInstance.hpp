#pragma once
#include "bort/CLI/CLIOptions.hpp"
#include "bort/IR/Module.hpp"

namespace bort {
  
struct BackendFatalError : std::runtime_error {
  using std::runtime_error::runtime_error;
};

class BackendInstance {
public:
  explicit BackendInstance(CLIOptions cliOptions, ir::Module module);
  void run();

private:
  CLIOptions m_CLIOptions;
  ir::Module m_Module;
};
} // namespace bort
