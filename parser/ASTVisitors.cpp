
#include <iostream>
#include <cassert>

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

    // Return nodes should always have one child, BUT
    // Due to the grammar structure, after building the AST for binary ops return will have two
    // children, a literal and a binary op node
    
    // This can be fixed by moving the literal value node to the bottom of the AST tree,
    // There will be a binary op node there with one child, so add it there
    if (node.children.size() == 2) {
        ID::ASTNodeId intconst_id = Arena<ASTNode>::invalid_id;
        ID::ASTNodeId binop_id = Arena<ASTNode>::invalid_id;

        auto intconst_iter = node.children.begin();

        for (auto it = node.children.begin(); it != node.children.end(); it++) {
            ID::ASTNodeId child = *it;
            if (ast.get_node(child)->node_type == AST_NODE_TYPE::INT_CONST_NODE) {
                intconst_id = child;
                intconst_iter = it;
            } else if (ast.get_node(child)->node_type == AST_NODE_TYPE::BINARY_OP_NODE) {
                binop_id = child;
            }
        }

        do {
            for (ID::ASTNodeId child : ast.get_node(binop_id)->children) {
                if (ast.get_node(child)->node_type == AST_NODE_TYPE::BINARY_OP_NODE) {
                    binop_id = child;
                    break;
                }
            } 
        } while (ast.get_node(binop_id)->children.size() != 1);

        ast.get_node(binop_id)->children.push_back(intconst_id);
        node.children.erase(intconst_iter, intconst_iter + 1);

        ast.get_node(intconst_id)->parent = binop_id;
    }

    for (auto it = node.children.begin(); it != node.children.end();) {
        ID::ASTNodeId child = *it;
        if (ast.get_node(child)->node_type == AST_NODE_TYPE::BINARY_OP_NODE && ast.get_node(child)->children.size() == 0) {
            it = node.children.erase(it, it + 1);
        } else {
            it++;
        }
    }
}

void ASTBuilderVisitor::visit(ASTTypeNode& node) {
    erase_children(node, AST_NODE_TYPE::TEMP_NODE);
}

void ASTBuilderVisitor::visit(ASTBinaryOpNode& node) {

    // Get operation for binary op node (if appliciable)
    for (auto it = node.children.begin(); it != node.children.end();) {
        ID::ASTNodeId child = *it;
        if (ast.get_node(child)->node_type == AST_NODE_TYPE::TEMP_NODE) {
            // If the binary op node contains a temp node child, the temp node defines the type of operation
            node.set_binary_op(dynamic_cast<ASTTempNode*>(ast.get_node(child).get())->data);
            it++;
        } else {
            it++;
        }
    }

    erase_children(node, AST_NODE_TYPE::TEMP_NODE);

    // Remove invalid binary op nodes
    // These are created as a result of the grammar having multiple rules to create one binary operation
    // To remove, just make node's children node's siblings, and delete the invalid node
    if (node.op == BINARY_OP::INVALID) {
        vector<ID::ASTNodeId>& children = node.children;
        vector<ID::ASTNodeId>& siblings = ast.get_node(node.parent)->children;

        siblings.insert(siblings.end(), children.begin(), children.end());
        children.clear();

        for (auto it = siblings.begin(); it != siblings.end();) {
            ID::ASTNodeId sibling = *it;
            if (ast.get_node(sibling)->node_type == AST_NODE_TYPE::BINARY_OP_NODE && dynamic_cast<ASTBinaryOpNode*>(ast.get_node(sibling).get())->op == BINARY_OP::INVALID) {
                it = siblings.erase(it, it + 1);
            } else {
                it++;
            }
        }
    }

    if (node.children.size() == 2) {
        if (ast.get_node(node.children.at(0))->node_type == AST_NODE_TYPE::INT_CONST_NODE && ast.get_node(node.children.at(1))->node_type == AST_NODE_TYPE::BINARY_OP_NODE) {
            std::swap(node.children.at(0), node.children.at(1));
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
    std::cout << "Binary op " << static_cast<uint32_t>(node.op) << std::endl;
}

void ASTPrinterVisitor::visit(ASTTempNode& node) {
    std::cout << "Temp " << node.data << std::endl;
}

void ASTPrinterVisitor::visit(ASTIdentNode& node) {
    std::cout << "Ident" << std::endl;

}

void ASTPrinterVisitor::visit(ASTIntConstNode& node) {
    std::cout << "Int const " << node.value << std::endl;
}