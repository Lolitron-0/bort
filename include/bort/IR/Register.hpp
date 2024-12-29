#pragma once
#include "bort/IR/Value.hpp"

namespace bort::ir {
class Register : public Value {
public:
  using Value::Value;
};
} // namespace bort::ir
