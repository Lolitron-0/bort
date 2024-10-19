#pragma once

#include "bort/Frontend/FrontendOptions.hpp"

namespace bort {

struct FrontendInstance {
public:
  explicit FrontendInstance(FrontendOptions frontendOptions);
  void Run();

private:
  FrontendOptions m_FrontendOptions;
};
} // namespace bort
