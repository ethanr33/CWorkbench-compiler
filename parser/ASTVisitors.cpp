
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

void ASTBuilderVisitor::erase_children(ASTNode& node, std::function<bool(ID::ASTNodeId)> condition) {
    for (auto it = node.children.begin(); it != node.children.end();) {
        ID::ASTNodeId child = *it;

        if (condition(child)) {
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
        ast.get_node(grandchild)->parent = parent;
        node_it++;
    }

    auto it = std::find(ast.get_node(parent)->children.begin(), ast.get_node(parent)->children.end(), node_id);
    ast.get_node(parent)->children.erase(it, it + 1);
}

void ASTBuilderVisitor::promote_children_on_condition(ID::ASTNodeId parent, std::function<bool(ID::ASTNodeId)> condition) {
    vector<ID::ASTNodeId> nodes_of_type;

    for (ID::ASTNodeId child : ast.get_node(parent)->children) {
        if (condition(child)) {
            nodes_of_type.push_back(child);
        }
    }

    for (int i = 0; i < nodes_of_type.size(); i++) {
        promote_children(nodes_of_type.at(i));
    }
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
    // Get rid of function_body nodes
    promote_children_by_type(AST_NODE_TYPE::TEMP_PARENT_NODE, node.id);

    erase_children(node, AST_NODE_TYPE::TEMP_NODE);
    erase_children(node, AST_NODE_TYPE::IDENT_NODE);
    erase_children(node, AST_NODE_TYPE::TYPE_NODE);

    promote_children_on_condition(node.id, [this](ID::ASTNodeId node_id) {
        if (ast.get_node(node_id)->node_type == AST_NODE_TYPE::TEMP_PARENT_NODE) {
            if (dynamic_cast<ASTTempParentNode*>(ast.get_node(node_id).get())->symbol == "<expression>") {
                return true;
            }
        }
        return false;
    });
}

void ASTBuilderVisitor::visit(ASTReturnNode& node) {

    // The rule for return is RETURN <expression> ;
    // Promote children of <expression> nodes to expose the underlying value we're returning

    assert(node.children.size() == 3);
    assert(ast.get_node(node.children.at(1))->node_type == AST_NODE_TYPE::TEMP_PARENT_NODE);

    promote_children(node.children.at(1));

    erase_children(node, AST_NODE_TYPE::TEMP_NODE);
}

void ASTBuilderVisitor::visit(ASTTypeNode& node) {
    erase_children(node, AST_NODE_TYPE::TEMP_NODE);
}

void ASTBuilderVisitor::visit(ASTBinaryOpNode& node) {

    promote_children_by_type(AST_NODE_TYPE::TEMP_PARENT_NODE, node.id);

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

    erase_children(node, [this](ID::ASTNodeId node_id) {
        if (ast.get_node(node_id)->node_type == AST_NODE_TYPE::BINARY_OP_NODE) {
            return ast.get_node(node_id)->children.size() == 0;
        } else {
            return false;
        }
    });

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
            if (ast.get_symbol_table().has_identifier(node.identifier)) {
                throw std::runtime_error("Redeclaration of variable " + node.identifier);
            } else {
                ID::SymbolTableId variable_id = ast.get_symbol_table().add_variable(node.parent, node.identifier);
            }

            break;
        default:
            if (!ast.get_symbol_table().has_identifier(node.identifier)) {
                throw std::runtime_error("Attempted to use identifier " + node.identifier + " before declaration");
            }
            break;
    }

    return;
}

void ASTBuilderVisitor::visit(ASTIntConstNode& node) {
    erase_children(node, AST_NODE_TYPE::TEMP_NODE);
}

void ASTBuilderVisitor::visit(ASTTempParentNode& node) {
    if (node.symbol == "<expression>") {
        promote_children_by_type(AST_NODE_TYPE::TEMP_PARENT_NODE, node.id);

        erase_children(node, [this](ID::ASTNodeId node_id) {
            if (ast.get_node(node_id)->node_type == AST_NODE_TYPE::BINARY_OP_NODE) {
                return ast.get_node(node_id)->children.size() == 0;
            } else {
                return false;
            }
        });

        // Expression node may contain two children: a literal and a binary op
        // This is not valid, so we need to put the literal inside of a binary op

        if (node.children.size() == 2) {
            // The rule for expression is either <literal> <binop> or IDENT <binop>, either way <binop> will be the second child of <expression>
            ID::ASTNodeId const_node = node.children.at(0);
            ID::ASTNodeId binop_node = node.children.at(1);

            assert(ast.get_node(const_node)->node_type == AST_NODE_TYPE::INT_CONST_NODE || ast.get_node(const_node)->node_type == AST_NODE_TYPE::IDENT_NODE);
            assert(ast.get_node(binop_node)->node_type == AST_NODE_TYPE::BINARY_OP_NODE);

            while (ast.get_node(binop_node)->children.size() > 1) {
                // Remember based on our rule above, binop will always be the second child
                binop_node = ast.get_node(binop_node)->children.at(1);
            }

            erase_children(node, [const_node](ID::ASTNodeId node_id) {
                return node_id == const_node;
            });

            ast.get_node(binop_node)->children.insert(ast.get_node(binop_node)->children.begin(), const_node);
            ast.get_node(const_node)->parent = binop_node;
        }

    } else if (node.symbol == "<functionbody>") {
        promote_children_by_type(AST_NODE_TYPE::TEMP_PARENT_NODE, node.id);
    }
}

void ASTBuilderVisitor::visit(ASTVariableDeclNode& node) {
    promote_children_on_condition(node.id, [this](ID::ASTNodeId node_id) {
        if (ast.get_node(node_id)->node_type == AST_NODE_TYPE::TEMP_PARENT_NODE) {
            if (dynamic_cast<ASTTempParentNode*>(ast.get_node(node_id).get())->symbol == "<variabledefinition>") {
                return true;
            }
        }
        return false;
    });

    promote_children_on_condition(node.id, [this](ID::ASTNodeId node_id) {
        if (ast.get_node(node_id)->node_type == AST_NODE_TYPE::TEMP_PARENT_NODE) {
            if (dynamic_cast<ASTTempParentNode*>(ast.get_node(node_id).get())->symbol == "<expression>") {
                return true;
            }
        }
        return false;
    });

    erase_children(node, AST_NODE_TYPE::TEMP_NODE);
    erase_children(node, AST_NODE_TYPE::TYPE_NODE);

    if (node.children.size() == 1) {
        node.defines_here = false;
    } else {
        node.defines_here = true;


    }
}

void ASTPrinterVisitor::visit(ASTRootNode& node) {
    std::cout << "(" << node.id << ") " << "Root" << std::endl;
}

void ASTPrinterVisitor::visit(ASTFunctionNode& node) {
    std::cout << "(" << node.id << ") " << "Function" << std::endl;
}

void ASTPrinterVisitor::visit(ASTReturnNode& node) {
    std::cout << "(" << node.id << ") " << "Return" << std::endl;
}

void ASTPrinterVisitor::visit(ASTTypeNode& node) {
    std::cout << "(" << node.id << ") " << "Type" << std::endl;
}

void ASTPrinterVisitor::visit(ASTBinaryOpNode& node) {
    std::cout << "(" << node.id << ") " << "Binary op " << static_cast<uint32_t>(node.op) << std::endl;
}

void ASTPrinterVisitor::visit(ASTTempNode& node) {
    std::cout << "(" << node.id << ") " << "Temp " << node.data << std::endl;
}

void ASTPrinterVisitor::visit(ASTIdentNode& node) {
    std::cout << "(" << node.id << ") " << "Ident " << node.identifier << std::endl;
}

void ASTPrinterVisitor::visit(ASTIntConstNode& node) {
    std::cout << "(" << node.id << ") " << "Int const " << node.value << std::endl;
}

void ASTPrinterVisitor::visit(ASTTempParentNode& node) {
    std::cout << "(" << node.id << ") " << "Temp parent " << node.symbol << std::endl;
}

void ASTPrinterVisitor::visit(ASTVariableDeclNode& node) {
    if (node.defines_here) {
        std::cout << "(" << node.id << ") " << "Variable definition" << std::endl;
    } else {
        std::cout << "(" << node.id << ") " << "Variable declaration" << std::endl;
    }
}