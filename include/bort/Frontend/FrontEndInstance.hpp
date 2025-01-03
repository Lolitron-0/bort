#pragma once
#include "bort/AST/ASTNode.hpp"
#include "bort/CLI/CLIOptions.hpp"

namespace bort {

class FrontEndFatalError : public std::runtime_error {
public:
  explicit FrontEndFatalError(const std::string& message);
};

struct FrontEndInstance {
public:
  explicit FrontEndInstance(CLIOptions cliOptions);
  auto run() -> Ref<ast::ASTRoot>;

private:
  CLIOptions m_CLIOptions;
};
} // namespace bort
