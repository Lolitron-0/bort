#pragma once
#include "bort/Lex/Token.hpp"

namespace bort::ast {

struct ASTDebugInfo {
  Token token; ///< Token node originated from
};

} // namespace bort::ast
