#pragma once
#include "bort/Frontend/Symbol.hpp"
#include "bort/Frontend/Type.hpp"
#include "bort/IR/Instruction.hpp"
#include "bort/IR/Value.hpp"

namespace bort::ir {

class CallInst final : public Instruction {
public:
  CallInst(Ref<Function> function, ValueRef destination,
           const std::vector<ValueRef>& args)
      : Instruction{ args.size(), std::move(destination) },
        m_ArgStartIndex{ 1 },
        m_Function{ std::move(function) },
        m_NumArgs{ args.size() } {
    for (size_t i{ 0 }; i < args.size(); i++) {
      setArg(i, args[i]);
    }
  }

  CallInst(Ref<Function> function, const std::vector<ValueRef>& args)
      : Instruction{ args.size() },
        m_ArgStartIndex{ 0 },
        m_Function{ std::move(function) },
        m_NumArgs{ args.size() } {
    for (size_t i{ 0 }; i < args.size(); i++) {
      setArg(i, args[i]);
    }
  }

  [[nodiscard]] auto isVoid() const -> bool {
    return m_Function->getReturnType()->is(TypeKind::Void);
  }

  [[nodiscard]] auto getFunction() const -> Ref<Function> {
    return m_Function;
  }

  [[nodiscard]] auto getArg(size_t index) const -> ValueRef {
    return m_Operands[m_ArgStartIndex + index];
  }

  [[nodiscard]] auto getNumArgs() const -> size_t {
    return m_NumArgs;
  }

  void setArg(size_t index, ValueRef value) {
    m_Operands[m_ArgStartIndex + index] = std::move(value);
  }

private:
  size_t m_ArgStartIndex;
  Ref<Function> m_Function;
  size_t m_NumArgs;
};

} // namespace bort::ir
