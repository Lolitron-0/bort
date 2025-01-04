#include "bort/IR/BasicBlock.hpp"
#include <fmt/format.h>

namespace bort::ir {

size_t BasicBlock::s_NameCounter{ 0 };

BasicBlock::BasicBlock(std::string name)
    : Value{ VoidType::get(), name.empty()
                                  ? fmt::format("L{}", s_NameCounter++)
                                  : std::move(name) } {
}

} // namespace bort::ir
