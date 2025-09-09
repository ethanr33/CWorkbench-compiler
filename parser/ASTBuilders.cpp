
#include "ASTBuilders.h"
#include "../codegen/SymbolTable.h"
#include "AST.h"

void ASTBuilderVisitor::visit(ASTRootNode& node) {
    return;
}

void ASTBuilderVisitor::visit(ASTFunctionNode& node) {
    for (auto it = node.children.begin(); it != node.children.end();) {
        ID::ASTNodeId child = *it;
        switch (ast.get_node(child)->node_type) {
            case AST_NODE_TYPE::IDENT_NODE:
                it = node.children.erase(it);
                break;
            case AST_NODE_TYPE::TEMP_NODE:
                it = node.children.erase(it);
                break;
            default:
                it++;
                break;
        }
    }
}

void ASTBuilderVisitor::visit(ASTReturnNode& node) {
    return;
}

void ASTBuilderVisitor::visit(ASTTypeNode& node) {
    return;
}

void ASTBuilderVisitor::visit(ASTBinaryOpNode& node) {
    return;
}

void ASTBuilderVisitor::visit(ASTTempNode& node) {
    return;
}

void ASTBuilderVisitor::visit(ASTIdentNode& node) {
    switch (ast.get_node(node.parent)->node_type) {
        case AST_NODE_TYPE::FUNCTION_NODE:
            ast.get_symbol_table().add(node.parent, node.identifier);
            break;
        default:
            break;
    }

    return;
}

void ASTBuilderVisitor::visit(ASTIntConstNode& node) {
    return;
}
