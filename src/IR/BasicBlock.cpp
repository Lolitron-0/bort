#include "bort/IR/BasicBlock.hpp"
#include "bort/IR/BranchInst.hpp"
#include <fmt/format.h>

namespace bort::ir {

size_t BasicBlock::s_NameCounter{ 0 };

BasicBlock::BasicBlock(std::string name)
    : Value{ VoidType::get(), name.empty()
                                  ? fmt::format(".L{}", s_NameCounter++)
                                  : std::move(name) } {
}

auto BasicBlock::getLast() const -> Ref<Instruction> {
  return m_Instructions.back();
}
auto BasicBlock::getLastAsBranch() const -> Ref<BranchInst> {
  return dynCastRef<BranchInst>(getLast());
}
} // namespace bort::ir
