#include "bort/IR/GlobalValue.hpp"

using namespace bort::ir;
using namespace bort;

std::unordered_map<Ref<Variable>, Ref<GlobalVariable>>
    GlobalVariable::m_Registry{};
