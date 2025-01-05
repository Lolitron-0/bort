#pragma once
#include "bort/IR/AllocaInst.hpp"
#include "bort/IR/BranchInst.hpp"
#include "bort/IR/LoadInst.hpp"
#include "bort/IR/OpInst.hpp"
#include <cul/cul.hpp>

namespace bort::ir {
using InstructionTypeList =
    cul::typelist::TypeList<AllocaInst, BranchInst, OpInst, LoadInst>;
} // namespace bort::ir
