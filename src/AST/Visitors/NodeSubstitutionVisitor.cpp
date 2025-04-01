#include "bort/AST/Visitors/NodeSubstitutionVisitor.hpp"
#include "bort/AST/NumberExpr.hpp"
#include "bort/AST/Visitors/ASTVisitor.hpp"
#include "bort/Lex/Token.hpp"

namespace bort::ast {

void NodeSubstitutionVisitor::visit(const Ref<UnaryOpExpr>& unaryOpNode) {
  StructureAwareASTVisitor::visit(unaryOpNode);

  // hacky, but allows to avoid maintaining non-trivial traversal in this
  // visitor (we would need to substitute node _operands_ instead of nodes
  // directly)
  if (unaryOpNode->getOp() == TokenKind::KW_sizeof) {
    auto operand{ unaryOpNode->getOperand() };
    auto newOperand{ getASTRoot()->registerNode<NumberExpr>(
        getASTRoot()->getNodeDebugInfo(operand),
        static_cast<NumberExpr::ValueT>(operand->getType()->getSizeof()),
        IntType::get()) };

    unaryOpNode->setOperand(std::move(newOperand));
    unaryOpNode->setOp(TokenKind::Plus);
  }
}

} // namespace bort::ast
