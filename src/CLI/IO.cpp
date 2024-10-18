#include "bort/CLI/IO.hpp"
#include <iostream>

namespace bort {

void EmitError(const std::string& message) {
  std::cerr << message << std::endl;
}

} // namespace bort
