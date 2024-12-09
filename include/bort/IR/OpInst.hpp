#pragma once
#include "bort/IR/Instruction.hpp"
#include "bort/IR/Value.hpp"
#include "bort/Lex/Token.hpp"
#include <utility>

namespace bort::ir {

class OpInst : public Instruction {
public:
  OpInst(TokenKind op, ValueRef dst, ValueRef src1, ValueRef src2)
      : Instruction{ std::move(dst) },
        m_Op(op),
        m_Src1(std::move(src1)),
        m_Src2(std::move(src2)) {
  }

  [[nodiscard]] auto getOp() const -> TokenKind {
    return m_Op;
  }
  [[nodiscard]] auto getSrc1() const -> const ValueRef& {
    return m_Src1;
  }
  [[nodiscard]] auto getSrc2() const -> const ValueRef& {
    return m_Src2;
  }

private:
  TokenKind m_Op;
  ValueRef m_Src1;
  ValueRef m_Src2;
};

} // namespace bort::ir
