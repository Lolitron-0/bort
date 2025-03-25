#pragma once
#include "bort/IR/Instruction.hpp"
#include "bort/IR/Value.hpp"
#include "bort/Lex/Token.hpp"
#include <utility>

namespace bort::ir {

class UnaryInst final : public Instruction {
public:
  UnaryInst(TokenKind op, ValueRef dst, ValueRef src)
      : Instruction{ 1, std::move(dst) },
        m_Op(op) {
    m_Operands[s_SrcIdx] = std::move(src);
  }

  [[nodiscard]] auto getOp() const -> TokenKind {
    return m_Op;
  }
  [[nodiscard]] auto getSrc() const -> ValueRef {
    return getOperand(s_SrcIdx);
  }

  void setSrc(ValueRef value) {
    m_Operands[s_SrcIdx] = std::move(value);
  }

private:
  static constexpr size_t s_SrcIdx{ 1 };
  TokenKind m_Op;
};

} // namespace bort::ir
