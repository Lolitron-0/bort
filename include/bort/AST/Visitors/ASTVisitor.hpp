#pragma once
#include "bort/AST/ASTDebugInfo.hpp"
#include "bort/Basic/Ref.hpp"
#include <utility>

namespace bort::ast {

class Node;
class NumberExpr;
class VariableExpr;
class InitializerList;
class IndexationExpr;
class StringExpr;
class CharExpr;
class BinOpExpr;
class UnaryOpExpr;
class VarDecl;
class Block;
class ASTRoot;
class FunctionDecl;
class FunctionCallExpr;
class ExpressionStmt;
class IfStmt;
class WhileStmt;
class ReturnStmt;
class BreakStmt;
class ContinueStmt;
class LabelStmt;
class GotoStmt;

class ASTVisitorBase {
public:
  virtual ~ASTVisitorBase() = default;

  [[nodiscard]] auto isASTInvalidated() const -> bool {
    return m_ASTInvalidated;
  }

protected:
  [[nodiscard]] inline auto getASTRoot() const -> const Ref<ASTRoot>& {
    return m_ASTRoot;
  }
  [[nodiscard]] auto getNodeDebugInfo(const Ref<Node>& node) const
      -> ASTDebugInfo;

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

/// \brief Base class of all SA visitors
///
/// Structure aware visitors act more like an AST pattern matching,
/// except they are a bit more low-level as they can perform operation
/// anywhere during the tree traversal. By default, non-overriden nodes
/// simply continue traversal with no action. Pre- and post-order visitors
/// can be easily implemented (and even mixed) using this interface
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
  virtual void visit(const Ref<BreakStmt>& /* breakNode */) {
    // leaf
  }
  virtual void visit(const Ref<ContinueStmt>& /* continueNode */) {
    // leaf
  }
  virtual void visit(const Ref<LabelStmt>& /* labelNode */) {
    // leaf
  }
  virtual void visit(const Ref<GotoStmt>& /* gotoNode */) {
    // leaf
  }
  virtual void visit(const Ref<VarDecl>& varDeclNode);
  virtual void visit(const Ref<InitializerList>& initializerListNode);
  virtual void visit(const Ref<IndexationExpr>& indexationExpr);
  virtual void visit(const Ref<FunctionDecl>& functionDeclNode);
  virtual void visit(const Ref<ExpressionStmt>& exprStmtNode);
  virtual void visit(const Ref<BinOpExpr>& binopNode);
  virtual void visit(const Ref<UnaryOpExpr>& unaryOpNode);
  virtual void visit(const Ref<Block>& blockNode);
  virtual void visit(const Ref<IfStmt>& ifStmtNode);
  virtual void visit(const Ref<WhileStmt>& whileStmtNode);
  virtual void visit(const Ref<ReturnStmt>& returnStmtNode);
  virtual void visit(const Ref<FunctionCallExpr>& /* functionCallNode */);
};

} // namespace bort::ast
