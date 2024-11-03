#pragma once
#include "bort/Basic/Ref.hpp"
#include <memory>

namespace bort::ast {

class NumberExpr;
class VariableExpr;
class StringExpr;
class CharExpr;
class BinOpExpr;
class Block;
class ASTRoot;

class ASTVisitor {
public:
  explicit ASTVisitor(Ref<ASTRoot> ast)
      : m_AST{ std::move(ast) } {
  }
  virtual ~ASTVisitor() = default;

  virtual void visit(ASTRoot* /* rootNode */) {
  }
  virtual void visit(NumberExpr* /* numNode */) {
  }
  virtual void visit(VariableExpr* /* varNode */) {
  }
  virtual void visit(StringExpr* /* strNode */) {
  }
  virtual void visit(CharExpr* /* charNode */) {
  }
  virtual void visit(BinOpExpr* /* binopNode */) {
  }
  virtual void visit(Block* /* blockNode */) {
  }

  [[nodiscard]] inline auto getASTRef() const -> const Ref<ASTRoot>& {
    return m_AST;
  }
  [[nodiscard]] auto isASTInvalidated() const -> bool {
    return m_ASTInvalidated;
  }

protected:
  void markASTInvalid() {
    m_ASTInvalidated = true;
  }

private:
  Ref<ASTRoot> m_AST;
  bool m_ASTInvalidated{ false };
};

/// Base class of all SA visitors, non-overriden nodes simply continue
/// traversal with no action
class StructureAwareASTVisitor
    : public ASTVisitor,
      public std::enable_shared_from_this<StructureAwareASTVisitor> {
protected:
  explicit StructureAwareASTVisitor(Ref<ASTRoot> ast)
      : ASTVisitor{ std::move(ast) } {
  }

public:
  void visit(ASTRoot* rootNode) override;

  void visit(NumberExpr* /* numNode */) override {
    // leaf
  }
  void visit(VariableExpr* /* varNode */) override {
    // leaf
  }
  void visit(StringExpr* /* strNode */) override {
    // leaf
  }
  void visit(CharExpr* /* charNode */) override {
    // leaf
  }
  void visit(BinOpExpr* binopNode) override;
  void visit(Block* blockNode) override;
};

} // namespace bort::ast
