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
          m_Operands[s_Src1Idx] = std::move(src1);
          m_Operands[s_Src2Idx] = std::move(src2);
  }

  [[nodiscard]] auto getOp() const -> TokenKind {
    return m_Op;
  }
  [[nodiscard]] auto getSrc1() const -> ValueRef {
    return getOperand(s_Src1Idx);
  }
  [[nodiscard]] auto getSrc2() const -> ValueRef {
    return getOperand(s_Src2Idx);
  }

private:
  static constexpr size_t s_Src1Idx{ 1 };
  static constexpr size_t s_Src2Idx{ 2 };
  TokenKind m_Op;
};

} // namespace bort::ir
