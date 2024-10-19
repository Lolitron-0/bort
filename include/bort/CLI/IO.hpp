#pragma once
#include <string>

namespace bort {

void EmitError(const std::string& message);
void EmitWarning(const std::string& message);

void DebugOut(const std::string& message);

#ifdef NDEBUG
#define DEBUG_OUT(...)
#else
#define DEBUG_OUT(message) ::bort::DebugOut((message))
#endif

} // namespace bort
