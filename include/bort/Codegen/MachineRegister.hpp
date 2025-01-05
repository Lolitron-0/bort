#pragma once
#include "bort/Frontend/Type.hpp"
#include "bort/IR/Value.hpp"
#include <array>

namespace bort::codegen {

template <typename T>
concept RegisterEnum = requires(T t) {
  { T::COUNT };
  std::is_enum_v<T>;
};

template <RegisterEnum RegEnum, std::string_view (*NameFunc)(RegEnum)>
class MachineRegister final : public ir::Value {
  explicit MachineRegister(RegEnum gprId)
      : Value{ IntType::get(), std::string{ NameFunc(gprId) } },
        m_GPRId{ gprId } {
  }

  static constexpr auto s_NumRegisters{ static_cast<size_t>(
      RegEnum::COUNT) };

public:
  static auto get(RegEnum gprId) -> Ref<MachineRegister> {
    static std::array<Ref<MachineRegister>, s_NumRegisters> s_Registry;
    static bool s_RegistryInitialized{ false };
    if (!s_RegistryInitialized) {
      for (int i{ 0 }; i < s_NumRegisters; i++) {
        s_Registry[i] = Ref<MachineRegister>{ new MachineRegister{
            static_cast<RegEnum>(i) } };
      }
      s_RegistryInitialized = true;
    }

    return s_Registry[static_cast<int>(gprId)];
  }

  [[nodiscard]] auto getGPRId() const -> RegEnum {
    return m_GPRId;
  }

private:
  RegEnum m_GPRId;
};
} // namespace bort::codegen
