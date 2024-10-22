#pragma once
#include "bort/Frontend/SourceFile.hpp"
#include <algorithm>
#include <string_view>

namespace bort {

auto startsWith(const SourceFileIt& pos, std::string_view prefix) -> bool;

void skipSpacesSince(SourceFileIt& pos);

auto consumeIdent(SourceFileIt& pos) -> std::string_view;


} // namespace bort
