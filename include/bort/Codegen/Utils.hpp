#pragma once
#include "bort/IR/BasicBlock.hpp"
#include "bort/IR/Metadata.hpp"
#include "bort/IR/Module.hpp"

namespace bort::codegen {

class FrameInfo : public ir::Metadata {
public:
  [[nodiscard]] auto toString() const -> std::string override;

  size_t Size;
  size_t NumVariables;
};

auto getUniqueOperands(const ir::IRFunction& func)
    -> std::vector<Ref<ir::Operand>>;

auto getUniqueOperands(const ir::BasicBlock& block)
    -> std::vector<Ref<ir::Operand>>;

auto isJumpInst(const Ref<ir::Instruction>& inst) -> bool;

} // namespace bort::codegen
