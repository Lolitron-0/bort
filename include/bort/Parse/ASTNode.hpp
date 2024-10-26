#pragma once
#include "bort/Lex/Token.hpp"
#include <memory>
#include <string>

namespace bort::ast {

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
public:
  explicit ExpressionNode(NodeKind kind)
      : Node{ kind } {
  }
};

class NumberExpr final : public ExpressionNode {
public:
  // TODO typed constants
  using ValueT = int;

  explicit NumberExpr(ValueT value)
      : ExpressionNode{ NodeKind::NumberExpr },
        m_Value{ value } {
  }

protected:
  void dump(int depth) const override;

private:
  ValueT m_Value;
};

class VariableExpr final : public ExpressionNode {
public:
  explicit VariableExpr(std::string name)
      : ExpressionNode{ NodeKind::VariableExpr },
        m_Name{ std::move(name) } {
  }

protected:
  void dump(int depth) const override;

private:
  std::string m_Name;
};

class BinOpExpr final : public ExpressionNode {
public:
  BinOpExpr(std::unique_ptr<Node> lhs, std::unique_ptr<Node> rhs,
            TokenKind op)
      : ExpressionNode{ NodeKind::BinOpExpr },
        m_Op{ op },
        m_Lhs{ std::move(lhs) },
        m_Rhs{ std::move(rhs) } {
  }

protected:
  void dump(int depth) const override;

private:
  TokenKind m_Op;
  std::unique_ptr<Node> m_Lhs;
  std::unique_ptr<Node> m_Rhs;
};

} // namespace bort::ast
