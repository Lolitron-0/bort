#pragma once
#include <string>

namespace bort {

void EmitError(const std::string& message);
void EmitWarning(const std::string& message);

void DebugOut(const std::string& message);

} // namespace bort
