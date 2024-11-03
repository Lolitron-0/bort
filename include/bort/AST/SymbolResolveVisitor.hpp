#pragma once
#include "bort/AST/ASTDebugInfo.hpp"
#include "bort/AST/ASTNode.hpp"
#include "bort/AST/ASTVisitor.hpp"
#include "bort/Basic/Assert.hpp"
#include "bort/Basic/Ref.hpp"
#include "bort/CLI/IO.hpp"
#include "bort/Frontend/Symbol.hpp"
#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>

namespace bort::ast {

class Scope {
public:
  explicit Scope(Ref<Scope> enclosingScope)
      : m_EnclosingScope{ std::move(enclosingScope) },
        m_Name{ std::nullopt } {
  }
  Scope(Ref<Scope> enclosingScope, std::string name)
      : m_EnclosingScope{ std::move(enclosingScope) },
        m_Name{ name.empty()
                    ? std::nullopt
                    : std::optional<std::string>{ std::move(name) } } {
  }

  [[nodiscard]] auto getEnclosingScope() const -> Ref<Scope> {
    return m_EnclosingScope;
  }
  [[nodiscard]] auto getName() const -> std::optional<std::string> {
    return m_Name;
  }
  void define(const Ref<Symbol>& symbol) {
    bort_assert(!m_SymbolMap.contains(symbol->getName()),
                "Symbol already defined");
    m_SymbolMap[symbol->getName()] = symbol;
  }
  [[nodiscard]] auto resolve(const std::string& name) -> Ref<Symbol> {
    if (m_SymbolMap.contains(name)) {
      return m_SymbolMap[name];
    }
    if (m_EnclosingScope) {
      return m_EnclosingScope->resolve(name);
    }
    return nullptr;
  }

private:
  Ref<Scope> m_EnclosingScope;
  std::optional<std::string> m_Name;
  std::unordered_map<std::string, Ref<Symbol>> m_SymbolMap;
};

class SymbolResolveVisitor final : public StructureAwareASTVisitor {
private:
  explicit SymbolResolveVisitor(Ref<ASTRoot> ast)
      : StructureAwareASTVisitor{ std::move(ast) },
        m_CurrentScope{ makeRef<Scope>(nullptr, "Global") } {
  }

public:
  static auto create(Ref<ASTRoot> ast) -> Ref<StructureAwareASTVisitor> {
    return Ref<SymbolResolveVisitor>{ new SymbolResolveVisitor{
        std::move(ast) } };
  }

  void visit(VariableExpr* varNode) override {
    if (varNode->isResolved()) {
      return;
    }
    auto symbol = resolve(varNode->getVarName());
    if (symbol) {
      varNode->setVariable(std::dynamic_pointer_cast<Variable>(symbol));
      return;
    }

    emitError(getASTRef()->getNodeDebugInfo(varNode).token,
              "Unresolved variable: {}", varNode->getVarName());
    markASTInvalid();
  }

  void visit(Block* blockNode) override {
    push();
    for (auto&& child : blockNode->getBody()) {
      child->structureAwareVisit(shared_from_this());
    }
    pop();
  }

private:
  void push() {
    push("");
  }
  void push(std::string name) {
    m_CurrentScope = makeRef<Scope>(m_CurrentScope, std::move(name));
  }
  void pop() {
    bort_assert(m_CurrentScope->getEnclosingScope() != nullptr,
                "Popping global scope");
    m_CurrentScope = m_CurrentScope->getEnclosingScope();
  }
  void define(const Ref<Symbol>& symbol) {
    m_CurrentScope->define(symbol);
  }
  [[nodiscard]] auto resolve(const std::string& name) -> Ref<Symbol> {
    return m_CurrentScope->resolve(name);
  }

  Ref<Scope> m_CurrentScope;
};
} // namespace bort::ast
