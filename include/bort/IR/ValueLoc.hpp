#pragma once
#include "bort/IR/Metadata.hpp"
#include <cstddef>
#include <string>

namespace bort::ir {

class ValueLoc {};

class StackLoc final : public ValueLoc, public Metadata {
public:
  explicit StackLoc(size_t offset)
      : m_Offset{ offset } {
  }

  [[nodiscard]] auto getOffset() const -> size_t {
    return m_Offset;
  }

  [[nodiscard]] auto toString() const -> std::string override;

private:
  size_t m_Offset;
};

/// \brief Register location
///
/// Implementation note: despite bort being RISC-V compiler, I want to
/// stick to a more generic framework for possible future use
class RegisterLoc final : public ValueLoc {
public:
  explicit RegisterLoc(int id)
      : m_RegisterId{ id } {
  }

  template <typename T = int>
  [[nodiscard]] auto getRegisterId() const -> T {
    return static_cast<T>(m_RegisterId);
  }

private:
  int m_RegisterId;
};

} // namespace bort::ir
