#pragma once
#include "bort/IR/Instruction.hpp"
#include "bort/IR/Value.hpp"
#include "bort/Lex/Token.hpp"
#include <utility>

namespace bort::ir {

class OpInst final : public Instruction {
public:
  OpInst(TokenKind op, ValueRef dst, ValueRef src1, ValueRef src2)
      : Instruction{ 2, std::move(dst) },
        m_Op(op) {
    m_Operands[Src1Idx] = std::move(src1);
    m_Operands[Src2Idx] = std::move(src2);
  }

  [[nodiscard]] auto getOp() const -> TokenKind {
    return m_Op;
  }
  [[nodiscard]] auto getSrc() const -> ValueRef {
    return getOperand(Src1Idx);
  }
  [[nodiscard]] auto getSrc2() const -> ValueRef {
    return getOperand(Src2Idx);
  }

  void setSrc(ValueRef value) {
    m_Operands[Src1Idx] = std::move(value);
  }
  void setSrc2(ValueRef value) {
    m_Operands[Src2Idx] = std::move(value);
  }

  static constexpr int Src1Idx{ 1 };
  static constexpr int Src2Idx{ 2 };

private:
  TokenKind m_Op;
};

} // namespace bort::ir
