#pragma once
#include "bort/Basic/Ref.hpp"
#include <memory>
#include <utility>

namespace bort::ast {

class Node;
class NumberExpr;
class VariableExpr;
class StringExpr;
class CharExpr;
class BinOpExpr;
class VarDecl;
class Block;
class ASTRoot;

class ASTVisitorBase {
public:
  virtual ~ASTVisitorBase() = default;

  [[nodiscard]] auto isASTInvalidated() const -> bool {
    return m_ASTInvalidated;
  }

protected:
  [[nodiscard]] inline auto getASTRef() const -> const Ref<ASTRoot>& {
    return m_ASTRoot;
  }
  void setASTRoot(Ref<ASTRoot> ast) {
    m_ASTRoot = std::move(ast);
  }

  void markASTInvalid() {
    m_ASTInvalidated = true;
  }

private:
  Ref<ASTRoot> m_ASTRoot{ nullptr };
  bool m_ASTInvalidated{ false };
};

class ASTVisitor : public ASTVisitorBase {
public:
  explicit ASTVisitor(Ref<ASTRoot> rootNode) {
    setASTRoot(std::move(rootNode));
  }

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
  virtual void visit(VarDecl* /* blockNode */) {
  }
};

/// \brief Base class of all SA visitors,
///
/// Structure aware visitors act more like an AST pattern matching,
/// except they are a bit more low-level (more flexible but
/// boilerplate-prone) they can perform operation anywhere in the
/// tree. However such visitors need to continue traversal themselves. By
/// default, non-overriden nodes simply continue traversal with no action.
class StructureAwareASTVisitor : public ASTVisitorBase {
public:
  void SAVisit(const Ref<ASTRoot>& node);

protected:
  void genericVisit(const Ref<Node>& node);

  virtual void visit(const Ref<ASTRoot>& rootNode);

  virtual void visit(const Ref<NumberExpr>& /* numNode */) {
    // leaf
  }
  virtual void visit(const Ref<VariableExpr>& /* varNode */) {
    // leaf
  }
  virtual void visit(const Ref<StringExpr>& /* strNode */) {
    // leaf
  }
  virtual void visit(const Ref<CharExpr>& /* charNode */) {
    // leaf
  }
  virtual void visit(const Ref<VarDecl>& /* varDeclNode */) {
    // leaf
  }
  virtual void visit(const Ref<BinOpExpr>& binopNode);
  virtual void visit(const Ref<Block>& blockNode);
};

} // namespace bort::ast
