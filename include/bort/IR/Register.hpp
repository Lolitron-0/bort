#pragma once
#include "bort/IR/Value.hpp"

namespace bort::ir {
class Register final : public Value {
  using Value::Value;

public:
  static auto create(TypeRef type) -> std::unique_ptr<Register> {
    static size_t nameCounter{ 0 };
    return std::unique_ptr<Register>(
        new Register{ std::move(type), std::to_string(nameCounter++) });
  }
};
} // namespace bort::ir
