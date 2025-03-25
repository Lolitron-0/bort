#include "bort/Codegen/ValueLoc.hpp"
#include "bort/Basic/Ref.hpp"
#include "bort/IR/Module.hpp"
#include "bort/IR/OpInst.hpp"
#include <fmt/format.h>
#include <string>

namespace bort::codegen {
ValueLoc::ValueLoc(LocationKind kind)
    : Value{ PointerType::get(VoidType::get()),
             std::string{ s_LocationKindToString.Find(kind).value_or(
                 "unknown_loc") } },
      m_Kind{ kind } {
}


} // namespace bort::codegen
