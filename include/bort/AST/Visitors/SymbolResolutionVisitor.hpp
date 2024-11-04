#pragma once
#include "bort/AST/Visitors/ASTVisitor.hpp"
#include "bort/Basic/Ref.hpp"
#include "bort/Frontend/Symbol.hpp"
#include <optional>
#include <stdexcept>
#include <unordered_map>

namespace bort::ast {

class SymbolAlreadyDefined : public std::runtime_error {
public:
  explicit SymbolAlreadyDefined(const std::string& name);
};

class Scope {
public:
  explicit Scope(Ref<Scope> enclosingScope);

  Scope(Ref<Scope> enclosingScope, std::string name);

  [[nodiscard]] auto getEnclosingScope() const -> Ref<Scope>;
  [[nodiscard]] auto getName() const -> std::optional<std::string>;
  void define(const Ref<Symbol>& symbol);
  [[nodiscard]] auto resolve(const std::string& name) -> Ref<Symbol>;

private:
  Ref<Scope> m_EnclosingScope;
  std::optional<std::string> m_Name;
  std::unordered_map<std::string, Ref<Symbol>> m_SymbolMap;
};

class SymbolResolutionVisitor final : public StructureAwareASTVisitor {
public:
  SymbolResolutionVisitor();

protected:
  void visit(const Ref<VariableExpr>& varNode) override;

  void visit(const Ref<VarDecl>& varDeclNode) override;

  void visit(const Ref<Block>& blockNode) override;

private:
  void push();
  void push(std::string name);
  void pop();
  void define(const Ref<Symbol>& symbol);
  [[nodiscard]] auto resolve(const std::string& name) -> Ref<Symbol>;

  Ref<Scope> m_CurrentScope;
};
} // namespace bort::ast
