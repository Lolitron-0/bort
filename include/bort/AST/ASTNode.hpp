#pragma once
#include "bort/AST/ASTDebugInfo.hpp"
#include "bort/AST/Visitors/ASTVisitor.hpp"
#include "bort/Basic/Assert.hpp"
#include "bort/Basic/Ref.hpp"
#include "bort/Frontend/Symbol.hpp"
#include <concepts>
#include <unordered_map>
#include <utility>

namespace bort::ast {

class ASTVisitor;

namespace internal {
template <typename T>
void dump(int depth, std::string_view name, T value);
} // namespace internal

/// For grammar description of each node refer to corresponding method in
/// Parser
enum class NodeKind {
  NumberExpr,
  VariableExpr,
  StringExpr,
  CharExpr,
  BinOpExpr,
  VarDecl,
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

  /// Applies operation for parent, then for children
  virtual void preOrderVisit(const Ref<ASTVisitor>& visitor) = 0;
  /// Applies operation for children, then for parent
  virtual void postOrderVisit(const Ref<ASTVisitor>& visitor) = 0;

  [[nodiscard]] auto getKind() const -> NodeKind {
    return m_Kind;
  }

  template <typename T>
  friend void internal::dump(int depth, std::string_view name, T value);

protected:
  NodeKind m_Kind;
};

class FunctionDecl : public Node {
public:
  FunctionDecl();

private:
  Ref<Function> m_Function;
  Ref<Block> m_Body;
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
    bort_assert(m_NodeDebugInfo.contains(node.get()),
                "No debug info for node");
    return m_NodeDebugInfo.at(node.get());
  }

  [[nodiscard]] auto getNodeDebugInfo(Node* node) -> ASTDebugInfo {
    bort_assert(m_NodeDebugInfo.contains(node), "No debug info for node");
    return m_NodeDebugInfo.at(node);
  }

  void pushChild(Ref<Node> child) {
    m_Children.push_back(std::move(child));
  }

  void preOrderVisit(const Ref<ASTVisitor>& visitor) override;
  void postOrderVisit(const Ref<ASTVisitor>& visitor) override;

  template <std::derived_from<Node> T, typename... Args>
  auto registerNode(ASTDebugInfo dbg, Args&&... args) -> Unique<T> {
    Unique<T> node{ new T{ std::forward<Args>(args)... } };
    m_NodeDebugInfo.insert(std::make_pair(node.get(), std::move(dbg)));
    return node;
  }

#if 0
  template <std::derived_from<Symbol> T, typename... Args>
  auto registerSymbol(ASTDebugInfo dbg, Args&&... args) -> Unique<T> {
    Unique<T> symbol{ new T{ std::forward<Args>(args)... } };
    m_NodeDebugInfo.insert(std::make_pair(symbol.get(), std::move(dbg)));
    return symbol;
  }
#endif

private:
  std::vector<Ref<Node>> m_Children;
  std::unordered_map<Node*, ASTDebugInfo> m_NodeDebugInfo;
};

} // namespace bort::ast