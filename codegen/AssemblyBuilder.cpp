#include "AssemblyBuilder.h"
#include "AST.h"

bool AssemblyBuilder::has_valid_entry_point() const {
    for (ID::ASTNodeId child : ast.get_node(ast.get_root_id())->children) {
        if (ast.get_node(child)->node_type == AST_NODE_TYPE::FUNCTION_NODE) {
            if (symbol_table.get(child).identifier == "main") {
                return true;
            }
        }
    }

    return false;
}

void AssemblyBuilder::validate_AST() const {
    if (!has_valid_entry_point()) {
        throw std::runtime_error("Program has no entry point main()");
    }
}

void AssemblyBuilder::visit(ASTRootNode& node) {
    generated_assembly += "[BITS 64]\nglobal _start\n_start:\n";
}

void AssemblyBuilder::visit(ASTFunctionNode& node) {

}

void AssemblyBuilder::visit(ASTReturnNode& node) {
    switch (ast.get_node(node.children.at(0))->node_type) {
        case AST_NODE_TYPE::INT_CONST_NODE:
            break;
        case AST_NODE_TYPE::BINARY_OP_NODE:
            break;
        default:
            throw std::runtime_error("Invalid return type in main()");
    }
}

void AssemblyBuilder::visit(ASTTypeNode& node) {

}

void AssemblyBuilder::visit(ASTBinaryOpNode& node) {

}

void AssemblyBuilder::visit(ASTTempNode& node) {

}

void AssemblyBuilder::visit(ASTIdentNode& node) {

}

void AssemblyBuilder::visit(ASTIntConstNode& node) {

}
