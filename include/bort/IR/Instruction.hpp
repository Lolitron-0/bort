#pragma once
#include "bort/IR/Value.hpp"

namespace bort::ir {

class Instruction : public Value {
public:
  [[nodiscard]] auto getDestination() const -> ValueRef {
    return m_Destination;
  }

protected:
  explicit Instruction(ValueRef destination)
      : Value{ destination->getType() },
        m_Destination{ std::move(destination) } {
  }
  explicit Instruction(TypeRef type)
      : Value{ std::move(type) } {
  }

private:
  ValueRef m_Destination;
};

} // namespace bort::ir
