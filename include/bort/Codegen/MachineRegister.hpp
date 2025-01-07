#pragma once
#include "bort/Frontend/Type.hpp"
#include "bort/IR/Value.hpp"

namespace bort::codegen {

class MachineRegister : public ir::Value {
protected:
  explicit MachineRegister(int gprId, std::string name)
      : Value{ IntType::get(), std::move(name) },
        m_GPRId{ gprId } {
  }

  [[nodiscard]] auto getGPRId() const -> int {
    return m_GPRId;
  }

private:
  int m_GPRId;
};

} // namespace bort::codegen
