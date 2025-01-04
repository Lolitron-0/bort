#include "bort/IR/Label.hpp"

namespace bort::ir {

size_t Label::s_NameCounter{ 0 };

Label::Label(std::string name)
    : m_Name{ name.empty() ? fmt::format("L{}", s_NameCounter++)
                           : std::move(name) } {
}

} // namespace bort::ir
