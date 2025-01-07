#pragma once
#include "bort/Basic/Ref.hpp"
#include "bort/IR/Value.hpp"
#include <unordered_set>

namespace bort::ir {
class Register final : public Operand {
  using Operand::Operand;

public:
  static auto getOrCreate(TypeRef type,
                          std::string name = "") -> Ref<Register> {
    static size_t s_NameCounter{ 0 };
    static std::unordered_set<Ref<Register>> s_Registers;
    if (name.empty()) {
      auto newReg{ Ref<Register>(new Register{
          std::move(type), std::to_string(s_NameCounter++) }) };
      s_Registers.insert(newReg);
      return newReg;
    }
    auto existingIt{ std::ranges::find_if(s_Registers, [&name](auto reg) {
      return reg->getName() == name;
    }) };
    if (existingIt != s_Registers.end()) {
      return *existingIt;
    }
    auto newReg{ Ref<Register>(
        new Register{ std::move(type), std::move(name) }) };
    s_Registers.insert(newReg);
    return newReg;
  }
};
} // namespace bort::ir
