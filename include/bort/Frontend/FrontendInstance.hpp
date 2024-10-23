#pragma once
#include "bort/Frontend/FrontendOptions.hpp"

namespace bort {

class FrontendFatalError : public std::runtime_error {
public:
  explicit FrontendFatalError(const std::string& message);
};

struct FrontendInstance {
public:
  explicit FrontendInstance(FrontendOptions frontendOptions);
  void run();

private:
  FrontendOptions m_CliOptions;
};
} // namespace bort
