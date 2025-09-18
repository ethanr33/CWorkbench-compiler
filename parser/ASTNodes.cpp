
#include "ASTNodes.h"
#include "ASTVisitors.h"

void ASTRootNode::visit(NodeVisitor& visitor) {
    visitor.visit(*this);
}

void ASTFunctionNode::visit(NodeVisitor& visitor) {
    visitor.visit(*this);
}

void ASTReturnNode::visit(NodeVisitor& visitor) {
    visitor.visit(*this);
}

void ASTTypeNode::visit(NodeVisitor& visitor) {
    visitor.visit(*this);
}

void ASTIntConstNode::visit(NodeVisitor& visitor) {
    visitor.visit(*this);
}

void ASTIdentNode::visit(NodeVisitor& visitor) {
    visitor.visit(*this);
}

void ASTBinaryOpNode::visit(NodeVisitor& visitor) {
    visitor.visit(*this);
}

void ASTTempNode::visit(NodeVisitor& visitor) {
    visitor.visit(*this);
}

void ASTTempParentNode::visit(NodeVisitor& visitor) {
    visitor.visit(*this);
}

void ASTVariableDeclNode::visit(NodeVisitor& visitor) {
    visitor.visit(*this);
}