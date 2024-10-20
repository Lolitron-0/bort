#pragma once

#include "bort/Frontend/FrontendOptions.hpp"

namespace bort {

struct FrontendInstance {
public:
  explicit FrontendInstance(FrontendOptions frontendOptions);
  void run();

private:
  FrontendOptions m_FrontendOptions;
};
} // namespace bort
