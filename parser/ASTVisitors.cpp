
#include <iostream>

#include "ASTVisitors.h"
#include "../codegen/SymbolTable.h"
#include "AST.h"

void ASTBuilderVisitor::erase_children(ASTNode& node, AST_NODE_TYPE node_type) {
    for (auto it = node.children.begin(); it != node.children.end();) {
        ID::ASTNodeId child = *it;

        if (ast.get_node(child)->node_type == node_type) {
            it = node.children.erase(it);
        } else {
            it++;
        }

    }
}

void ASTBuilderVisitor::visit(ASTRootNode& node) {
    erase_children(node, AST_NODE_TYPE::TEMP_NODE);
}

void ASTBuilderVisitor::visit(ASTFunctionNode& node) {
    erase_children(node, AST_NODE_TYPE::TEMP_NODE);
    erase_children(node, AST_NODE_TYPE::IDENT_NODE);
}

void ASTBuilderVisitor::visit(ASTReturnNode& node) {
    erase_children(node, AST_NODE_TYPE::TEMP_NODE);
}

void ASTBuilderVisitor::visit(ASTTypeNode& node) {
    erase_children(node, AST_NODE_TYPE::TEMP_NODE);
}

void ASTBuilderVisitor::visit(ASTBinaryOpNode& node) {
    erase_children(node, AST_NODE_TYPE::TEMP_NODE);

    for (auto it = node.children.begin(); it != node.children.end();) {
        ID::ASTNodeId child = *it;
        if (ast.get_node(child)->node_type == AST_NODE_TYPE::BINARY_OP_NODE && ast.get_node(child)->children.size() == 0) {
            node.children.erase(it);
        } else {
            it++;
        }
    }
}

void ASTBuilderVisitor::visit(ASTTempNode& node) {
    erase_children(node, AST_NODE_TYPE::TEMP_NODE);
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
    erase_children(node, AST_NODE_TYPE::TEMP_NODE);
}

void ASTPrinterVisitor::visit(ASTRootNode& node) {
    std::cout << "Root" << std::endl;
}

void ASTPrinterVisitor::visit(ASTFunctionNode& node) {
    std::cout << "Function" << std::endl;
}

void ASTPrinterVisitor::visit(ASTReturnNode& node) {
    std::cout << "Return" << std::endl;
}

void ASTPrinterVisitor::visit(ASTTypeNode& node) {
    std::cout << "Type" << std::endl;
}

void ASTPrinterVisitor::visit(ASTBinaryOpNode& node) {
    std::cout << "Binary op" << std::endl;
}

void ASTPrinterVisitor::visit(ASTTempNode& node) {
    std::cout << "Temp" << std::endl;
}

void ASTPrinterVisitor::visit(ASTIdentNode& node) {
    std::cout << "Ident" << std::endl;

}

void ASTPrinterVisitor::visit(ASTIntConstNode& node) {
    std::cout << "Int const" << std::endl;
}