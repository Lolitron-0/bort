#include "bort/AST/Visitors/SymbolResolutionVisitor.hpp"
#include "bort/AST/Block.hpp"
#include "bort/AST/VarDecl.hpp"
#include "bort/AST/VariableExpr.hpp"
#include "bort/AST/Visitors/ASTVisitor.hpp"
#include "bort/CLI/IO.hpp"

namespace bort::ast {

Scope::Scope(Ref<Scope> enclosingScope)
    : m_EnclosingScope{ std::move(enclosingScope) },
      m_Name{ std::nullopt } {
}

Scope::Scope(Ref<Scope> enclosingScope, std::string name)
    : m_EnclosingScope{ std::move(enclosingScope) },
      m_Name{ name.empty()
                  ? std::nullopt
                  : std::optional<std::string>{ std::move(name) } } {
}

auto Scope::getEnclosingScope() const -> Ref<Scope> {
  return m_EnclosingScope;
}

auto Scope::getName() const -> std::optional<std::string> {
  return m_Name;
}

void Scope::define(const Ref<Symbol>& symbol) {
  if (m_SymbolMap.contains(symbol->getName())) {
    throw SymbolAlreadyDefined{ symbol->getName() };
  }
  m_SymbolMap[symbol->getName()] = symbol;
}

auto Scope::resolve(const std::string& name) -> Ref<Symbol> {
  if (m_SymbolMap.contains(name)) {
    return m_SymbolMap[name];
  }
  if (m_EnclosingScope) {
    return m_EnclosingScope->resolve(name);
  }
  return nullptr;
}

SymbolResolutionVisitor::SymbolResolutionVisitor()
    : m_CurrentScope{ makeRef<Scope>(nullptr, "Global") } {
}

void SymbolResolutionVisitor::visit(const Ref<VariableExpr>& varNode) {
  if (varNode->isResolved()) {
    return;
  }
  auto symbol = resolve(varNode->getVarName());
  if (symbol) {
    varNode->setVariable(std::dynamic_pointer_cast<Variable>(symbol));
    return;
  }

  Diagnostic::emitError(getASTRoot()->getNodeDebugInfo(varNode).token,
                        "Unresolved variable: {}", varNode->getVarName());
  markASTInvalid();
}

void SymbolResolutionVisitor::visit(const Ref<VarDecl>& varDeclNode) {
  try {
    define(varDeclNode->getVariable());
  } catch (const SymbolAlreadyDefined& e) {
    Diagnostic::emitError(
        getASTRoot()->getNodeDebugInfo(varDeclNode).token, "{}", e.what());
    auto prevSym{ resolve(varDeclNode->getVariable()->getName()) };
    bort_assert(prevSym, "define/resolve mismatch");
    Diagnostic::emitWarning("Previously defined as: [ {} ]",
                            prevSym->toString());
    markASTInvalid();
  }
}

void SymbolResolutionVisitor::visit(const Ref<Block>& blockNode) {
  push();
  for (auto&& child : blockNode->getBody()) {
    genericVisit(child);
  }
  pop();
}

void SymbolResolutionVisitor::push() {
  push("");
}

void SymbolResolutionVisitor::push(std::string name) {
  m_CurrentScope = makeRef<Scope>(m_CurrentScope, std::move(name));
}

void SymbolResolutionVisitor::pop() {
  bort_assert(m_CurrentScope->getEnclosingScope() != nullptr,
              "Popping global scope");
  m_CurrentScope = m_CurrentScope->getEnclosingScope();
}

void SymbolResolutionVisitor::define(const Ref<Symbol>& symbol) {
  m_CurrentScope->define(symbol);
}

auto SymbolResolutionVisitor::resolve(const std::string& name)
    -> Ref<Symbol> {
  return m_CurrentScope->resolve(name);
}

SymbolAlreadyDefined::SymbolAlreadyDefined(const std::string& name)
    : std::runtime_error{ fmt::format("Redifinition of {}", name) } {
}
} // namespace bort::ast
