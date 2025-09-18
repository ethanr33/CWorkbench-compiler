
#include <iostream>
#include <cassert>
#include <algorithm>

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

// Node has to be a child of the current iterating node
void ASTBuilderVisitor::promote_children(ID::ASTNodeId node_id) {
    ID::ASTNodeId parent = ast.get_node(node_id)->parent;

    auto node_it = std::find(ast.get_node(parent)->children.begin(), ast.get_node(parent)->children.end(), node_id);

    for (ID::ASTNodeId grandchild : ast.get_node(node_id)->children) {
        node_it = ast.get_node(parent)->children.insert(node_it, grandchild);
    }

    auto it = std::find(ast.get_node(parent)->children.begin(), ast.get_node(parent)->children.end(), node_id);
    ast.get_node(parent)->children.erase(it, it + 1);
}

void ASTBuilderVisitor::promote_children_by_type(AST_NODE_TYPE type, ID::ASTNodeId parent) {
    vector<ID::ASTNodeId> nodes_of_type;

    for (ID::ASTNodeId child : ast.get_node(parent)->children) {
        if (ast.get_node(child)->node_type == type) {
            nodes_of_type.push_back(child);
        }
    }

    for (int i = 0; i < nodes_of_type.size(); i++) {
        promote_children(nodes_of_type.at(i));
    }
}

void ASTBuilderVisitor::visit(ASTRootNode& node) {
    erase_children(node, AST_NODE_TYPE::TEMP_NODE);
}

void ASTBuilderVisitor::visit(ASTFunctionNode& node) {
    erase_children(node, AST_NODE_TYPE::TEMP_NODE);
    erase_children(node, AST_NODE_TYPE::IDENT_NODE);

    promote_children_by_type(AST_NODE_TYPE::TEMP_PARENT_NODE, node.id);
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

    for (ID::ASTNodeId child : node.children) {
        if (ast.get_node(child)->node_type == AST_NODE_TYPE::TEMP_PARENT_NODE) {
            promote_children(child);
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
    // if (node.op == BINARY_OP::INVALID) {
    //     vector<ID::ASTNodeId>& children = node.children;
    //     vector<ID::ASTNodeId>& siblings = ast.get_node(node.parent)->children;

    //     siblings.insert(siblings.end(), children.begin(), children.end());
    //     children.clear();

    //     for (auto it = siblings.begin(); it != siblings.end();) {
    //         ID::ASTNodeId sibling = *it;
    //         if (ast.get_node(sibling)->node_type == AST_NODE_TYPE::BINARY_OP_NODE && dynamic_cast<ASTBinaryOpNode*>(ast.get_node(sibling).get())->op == BINARY_OP::INVALID) {
    //             it = siblings.erase(it, it + 1);
    //         } else {
    //             it++;
    //         }
    //     }
    // }

    promote_children_by_type(AST_NODE_TYPE::TEMP_PARENT_NODE, node.id);

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
            ast.get_symbol_table().add_function(node.parent, node.identifier);
            break;
        case AST_NODE_TYPE::VARIABLE_DECL_NODE:
            ast.get_symbol_table().add_variable(node.parent, node.identifier);
        default:
            break;
    }

    return;
}

void ASTBuilderVisitor::visit(ASTIntConstNode& node) {
    erase_children(node, AST_NODE_TYPE::TEMP_NODE);
}

void ASTBuilderVisitor::visit(ASTTempParentNode& node) {
    if (node.symbol == "<expression>") {
        if (node.children.size() == 2) {
                ID::ASTNodeId first = node.children.at(0);
                ID::ASTNodeId second = node.children.at(1);
                if ((ast.get_node(first)->node_type == AST_NODE_TYPE::INT_CONST_NODE || ast.get_node(first)->node_type == AST_NODE_TYPE::IDENT_NODE) && ast.get_node(second)->node_type == AST_NODE_TYPE::BINARY_OP_NODE) {
                    ast.get_node(second)->children.push_back(first);
                    ast.get_node(first)->parent = second;
                    node.children.erase(node.children.begin(), node.children.begin() + 1);
                }
        }

    }
}

void ASTBuilderVisitor::visit(ASTVariableDeclNode& node) {
    erase_children(node, AST_NODE_TYPE::TEMP_NODE);
    erase_children(node, AST_NODE_TYPE::TYPE_NODE);

    for (ID::ASTNodeId child : node.children) {
        if (ast.get_node(child)->node_type == AST_NODE_TYPE::TEMP_PARENT_NODE) {
            promote_children(child);
        }
    }
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

void ASTPrinterVisitor::visit(ASTTempParentNode& node) {
    std::cout << "Temp parent " << node.symbol << std::endl;
}

void ASTPrinterVisitor::visit(ASTVariableDeclNode& node) {
    if (node.has_definition) {
        std::cout << "Variable definition" << std::endl;
    } else {
        std::cout << "Variable declaration" << std::endl;
    }
}