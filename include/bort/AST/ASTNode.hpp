#pragma once
#include "bort/AST/ASTDebugInfo.hpp"
#include "bort/Basic/Assert.hpp"
#include "bort/Basic/Ref.hpp"
#include "bort/Frontend/Symbol.hpp"
#include "bort/Frontend/Type.hpp"
#include "bort/Lex/Token.hpp"
#include <memory>
#include <unordered_map>
#include <utility>

namespace bort::ast {

class ASTVisitor;

namespace internal {
template <typename T>
void dump(int depth, std::string_view name, T value);
} // namespace internal

enum class NodeKind {
  NumberExpr,
  VariableExpr,
  StringExpr,
  CharExpr,
  BinOpExpr,
  Block,
  ASTRoot,
  NUM_NODES
};

class Node {
public:
  virtual ~Node() = default;
  explicit Node(NodeKind kind)
      : m_Kind{ kind } {
  }

  void dump() const {
    dump(0);
  }

  /// Applies operation for parent, then for children
  virtual void preOrderVisit(const Ref<ASTVisitor>& visitor) = 0;
  /// Applies operation for children, then for parent
  virtual void postOrderVisit(const Ref<ASTVisitor>& visitor) = 0;
  /// \brief Handles special type of visitors
  ///
  /// Structure aware visitors act more like an AST pattern matching,
  /// except they are a bit more low-level (more flexible but
  /// boilerplate-causing) they can perform operation anywhere in the
  /// tree. However such visitors need to continue traversal themselves.
  void structureAwareVisit(const Ref<ASTVisitor>& visitor);

  [[nodiscard]] auto getKind() const -> NodeKind {
    return m_Kind;
  }

  template <typename T>
  friend void internal::dump(int depth, std::string_view name, T value);

protected:
  virtual void dump(int depth) const;

  NodeKind m_Kind;
};

class ExpressionNode : public Node {
protected:
  // in case expression type is not yet known
  explicit ExpressionNode(NodeKind kind)
      : ExpressionNode{ kind, nullptr } {
  }
  ExpressionNode(NodeKind kind, TypeRef type)
      : Node{ kind },
        m_Type{ std::move(type) } {
  }

public:
  [[nodiscard]] auto isTypeResolved() const -> bool {
    return m_Type != nullptr;
  }

  [[nodiscard]] auto getType() const -> TypeRef {
    return m_Type;
  }

  using Node::dump;

protected:
  void dump(int depth) const override;
  TypeRef m_Type;
};

class NumberExpr final : public ExpressionNode {
public:
  // TODO typed constants
  using ValueT = int;

private:
  explicit NumberExpr(ValueT value)
      : ExpressionNode{ NodeKind::NumberExpr, IntType::get() },
        m_Value{ value } {
  }

public:
  void preOrderVisit(const Ref<ASTVisitor>& visitor) override;
  void postOrderVisit(const Ref<ASTVisitor>& visitor) override;

  friend class ASTRoot;

protected:
  void dump(int depth) const override;

private:
  ValueT m_Value;
};

class VariableExpr final : public ExpressionNode {
  explicit VariableExpr(const Ref<Symbol>& variable)
      : ExpressionNode{ NodeKind::VariableExpr } {
    bort_assert((variable->getKind() == ObjectKind::Variable),
                "Variable expr got not variable object");
    m_Variable = std::dynamic_pointer_cast<Variable>(variable);
    m_Type = m_Variable->isShallow() ? nullptr : m_Variable->getType();
  }

public:
  [[nodiscard]] auto getVariable() const -> Ref<Variable> {
    return m_Variable;
  }
  [[nodiscard]] auto getVarName() const -> std::string {
    return m_Variable->getName();
  }

  [[nodiscard]] auto isResolved() const -> bool {
    return !m_Variable->isShallow();
  }
  void setVariable(Ref<Variable> variable) {
    m_Variable = std::move(variable);
  }

  void preOrderVisit(const Ref<ASTVisitor>& visitor) override;
  void postOrderVisit(const Ref<ASTVisitor>& visitor) override;

  friend class ASTRoot;

protected:
  void dump(int depth) const override;

private:
  Ref<Variable> m_Variable;
};

class BinOpExpr final : public ExpressionNode {
  BinOpExpr(Ref<Node> lhs, Ref<Node> rhs, TokenKind op)
      : ExpressionNode{ NodeKind::BinOpExpr, nullptr },
        m_Op{ op },
        m_Lhs{ std::move(lhs) },
        m_Rhs{ std::move(rhs) } {
  }

public:
  void preOrderVisit(const Ref<ASTVisitor>& visitor) override;
  void postOrderVisit(const Ref<ASTVisitor>& visitor) override;

  [[nodiscard]] auto getOp() const -> TokenKind {
    return m_Op;
  }
  [[nodiscard]] auto getLhs() const -> Ref<Node> {
    return m_Lhs;
  }
  [[nodiscard]] auto getRhs() const -> Ref<Node> {
    return m_Rhs;
  }

  friend class ASTRoot;

protected:
  void dump(int depth) const override;

private:
  TokenKind m_Op;
  Ref<Node> m_Lhs;
  Ref<Node> m_Rhs;
};

class Block final : public Node {
  Block()
      : Node{ NodeKind::Block } {
  }

public:
  [[nodiscard]] auto getBody() const -> const std::vector<Ref<Node>>& {
    return m_Body;
  }
  void pushChild(Ref<Node> child) {
    m_Body.push_back(std::move(child));
  }

  void preOrderVisit(const Ref<ASTVisitor>& visitor) override;
  void postOrderVisit(const Ref<ASTVisitor>& visitor) override;

  friend class ASTRoot;

protected:
  void dump(int depth) const override;

private:
  std::vector<Ref<Node>> m_Body;
};

/// This is an AST itself with some convenience methods related to whole
/// tree
class ASTRoot final : public Node {
public:
  ASTRoot()
      : Node{ NodeKind::ASTRoot } {
  }

  [[nodiscard]] auto getChildren() const
      -> const std::vector<Ref<Node>>& {
    return m_Children;
  }
  [[nodiscard]] auto getNodeDebugInfo(const Ref<Node>& node)
      -> ASTDebugInfo {
    bort_assert(m_DebugInfo.contains(node.get()),
                "No debug info for node");
    return m_DebugInfo.at(node.get());
  }

  [[nodiscard]] auto getNodeDebugInfo(Node* node) -> ASTDebugInfo {
    bort_assert(m_DebugInfo.contains(node), "No debug info for node");
    return m_DebugInfo.at(node);
  }

  void pushChild(Ref<Node> child) {
    m_Children.push_back(std::move(child));
  }

  void preOrderVisit(const Ref<ASTVisitor>& visitor) override;
  void postOrderVisit(const Ref<ASTVisitor>& visitor) override;

  template <typename T, typename... Args>
  auto registerNode(Token loc, Args&&... args) -> Unique<T> {
    auto node{ Unique<T>{ new T{ std::forward<Args>(args)... } } };
    m_DebugInfo.insert(
        std::make_pair(node.get(), ASTDebugInfo{ std::move(loc) }));
    return node;
  }

  using Node::dump;

protected:
  void dump(int depth) const override;

private:
  std::vector<Ref<Node>> m_Children;
  std::unordered_map<Node*, ASTDebugInfo> m_DebugInfo;
};

} // namespace bort::ast
