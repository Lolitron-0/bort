#pragma once
#include "bort/Codegen/MachineRegister.hpp"
#include "bort/IR/Metadata.hpp"
#include <cstddef>
#include <string>

namespace bort::codegen {

enum class LocationKind {
  Stack,
  Register,
  Global
};

class ValueLoc {
public:
  virtual ~ValueLoc() = default;

  [[nodiscard]] auto getKind() const -> LocationKind {
    return m_Kind;
  }

protected:
  explicit ValueLoc(LocationKind kind)
      : m_Kind{ kind } {
  }

private:
  LocationKind m_Kind;
};

class StackLoc final : public ValueLoc {
public:
  explicit StackLoc(size_t offset)
      : ValueLoc{ LocationKind::Stack },
        m_Offset{ offset } {
  }

  [[nodiscard]] auto getOffset() const -> size_t {
    return m_Offset;
  }

private:
  size_t m_Offset;
};

/// \brief Register location
///
/// Implementation note: despite bort being RISC-V compiler, I want to
/// stick to a more generic framework for possible future use
class RegisterLoc final : public ValueLoc {
public:
  explicit RegisterLoc(Ref<MachineRegister> reg)
      : ValueLoc{ LocationKind::Register },
        m_Register{ std::move(reg) } {
  }

  [[nodiscard]] auto getRegister() const -> Ref<MachineRegister> {
    return m_Register;
  }

private:
  Ref<MachineRegister> m_Register;
};

} // namespace bort::codegen
