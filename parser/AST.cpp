
#include <stdexcept>
#include <iostream>
#include <variant>
#include <algorithm>
#include <cassert>

#include "AST.h"
#include "../tools/debug.h"

using std::stack;
using std::variant;
using std::get_if;
using std::get;
using std::runtime_error;

ID::ASTNodeId AST::get_root_id() {
    return root;
}

std::unique_ptr<ASTNode>& AST::get_node(ID::ASTNodeId id) {
    return node_arena.get(id);
}

const std::unique_ptr<ASTNode>& AST::get_node(ID::ASTNodeId id) const {
   return node_arena.get(id);
}

ID::ASTNodeId AST::add_node(std::unique_ptr<ASTNode>&& new_node) {
    ID::ASTNodeId id = node_arena.add(std::move(new_node));
    node_arena.get(id)->id = id;
    return id;
}

void AST::remove_node(ID::ASTNodeId node_id) {
    ID::ASTNodeId parent = get_node(node_id)->parent;
    for (auto it = get_node(parent)->children.begin(); it != get_node(parent)->children.end();) {
        if (*it == node_id) {
            it = get_node(parent)->children.erase(it, it + 1);
        } else {
            it++;
        }
    }
}

SymbolTable& AST::get_symbol_table() {
    return symbol_table;
}

const SymbolTable& AST::get_symbol_table() const {
    return symbol_table;
}

void AST::print_AST_helper(ID::ASTNodeId node, int level) {
    
    for (int i = 0; i < level; i++) {
        std::cout << "  ";
    }

    node_arena.get(node)->visit(ast_printer);

    for (ID::ASTNodeId child : node_arena.get(node)->children) {
        print_AST_helper(child, level + 1);
    }
}

void AST::print_AST() {
    print_AST_helper(root, 0);
}

void AST::construct_AST_helper(ID::ASTNodeId node) {
    for (ID::ASTNodeId child : node_arena.get(node)->children) {
        construct_AST_helper(child);
    }

    node_arena.get(node)->visit(ast_builder);
}

void AST::construct_AST_from_parse_tree() {
    construct_AST_helper(root);
    construct_expression_trees();
}

void AST::construct_leaf_node(Token& token, SymbolId symbol, stack<ID::ASTNodeId>& ast_stack) {
    ID::ASTNodeId id;
    switch (grammar.symbol_arena.get(symbol).corresponding_token) {
        case TOKEN_TYPE::INT_CONST:
            id = add_node(std::make_unique<ASTIntConstNode>(std::stoi(token.value)));
            break;
        case TOKEN_TYPE::IDENTIFIER:
            id = add_node(std::make_unique<ASTIdentNode>(token.value));
            break;
        default:
            id = add_node(std::make_unique<ASTTempNode>(grammar.symbol_arena.get(symbol).symbol, grammar.symbol_arena.get(symbol).corresponding_token));
            break;
    }

    ast_stack.push(id);
}

bool AST::construct_production_node(int production_index, stack<ID::ASTNodeId>& ast_stack) {
    Rule rule = grammar.productions.at(production_index);
    string production_symbol =  grammar.symbol_arena.get(rule.symbol).symbol;

    ID::ASTNodeId parent_node = Arena<ID::ASTNodeId>::invalid_id;

    if (production_symbol == "<root>") {
        parent_node = add_node(std::make_unique<ASTRootNode>());
    } else if (production_symbol == "<function>") {
        parent_node = add_node(std::make_unique<ASTFunctionNode>());
    } else if (production_symbol == "<return>") {
        parent_node = add_node(std::make_unique<ASTReturnNode>());
    } else if (production_symbol == "<type>") {
        parent_node = add_node(std::make_unique<ASTTypeNode>());
    } else if (production_symbol == "<binaryop>") {
        parent_node = add_node(std::make_unique<ASTBinaryOpNode>());
    } else if (production_symbol == "<variabledeclaration>") {
        parent_node = add_node(std::make_unique<ASTVariableDeclNode>());
    } else {
        parent_node = add_node(std::make_unique<ASTTempParentNode>(production_symbol));
    }

    if (parent_node == Arena<ID::ASTNodeId>::invalid_id) {
        return false;
    } 
    
    for (int i = 0; i < rule.production_rule.size(); i++) {
        if (rule.production_rule.at(i) == grammar.symbols.at("ε")) {
            continue;
        }

        ID::ASTNodeId child = ast_stack.top();
        node_arena.get(child)->parent = parent_node;

        node_arena.get(parent_node)->children.push_back(child);
        ast_stack.pop();
    }

    // Tokens are popped off in reverse order from the stack,
    // So reverse the children array to maintain the original ordering
    std::reverse(node_arena.get(parent_node)->children.begin(), node_arena.get(parent_node)->children.end());

    ast_stack.push(parent_node);
    
    return true;
}

void AST::construct_parse_tree(const vector<Token>& token_stream) {
    stack<ID::ASTNodeId> ast_stack;
    stack<std::variant<SymbolId, int>> symbol_stack;

    int i = 0;

    symbol_stack.push(grammar.symbols.at("$"));
    symbol_stack.push(grammar.symbols.at("<root>"));

    while (true) {

        if (get_if<SymbolId>(&symbol_stack.top())) {
            SymbolId stack_top = *get_if<SymbolId>(&symbol_stack.top());

            if (grammar.symbol_arena.get(stack_top).is_terminal) {
                // Check to see if we have reached end of input and we have reached the end of input token as well
                if (stack_top == grammar.symbols.at("$")) {
                    // If for some reason we reach the end of input symbol on the stack but there are still tokens left to use, then there is a syntax error
                    if (i == token_stream.size() - 1) {
                        break;
                    } else {
                        throw runtime_error("Syntax error");
                    }
                }

                // If terminal symbol, consume token from the input string and pop
                Token cur_token = token_stream.at(i);
                
                // Check if out token's TOKEN_TYPE matches with the stack top's symbol's TOKEN_TYPE
                if (TERMINAL_MAP.at(grammar.symbol_arena.get(stack_top).symbol) == cur_token.token_type) {
                    // Consume input symbol and pop from stack
                    i++;

                    construct_leaf_node(cur_token, stack_top, ast_stack);
                    symbol_stack.pop();
                } else if (stack_top == grammar.symbols.at("ε")) {
                    // If epsilon appears on the stack, skip it because epsilon is the empty string
                    symbol_stack.pop();   
                } else {
                    throw runtime_error("Syntax error");
                }
            } else {
                // If terminal symbol, consume token from the input string and pop
                Token cur_token = token_stream.at(i);
                
                SymbolId cur_symbol = stack_top;
                SymbolId next_token_symbol = grammar.token_to_symbol(cur_token);

                if (grammar.parse_table.find(cur_symbol) == grammar.parse_table.end()) {
                    throw runtime_error("Syntax error");
                }

                if (grammar.parse_table.at(cur_symbol).find(next_token_symbol) == grammar.parse_table.at(cur_symbol).end()) {
                    throw runtime_error("Syntax error");
                }

                int production_number = grammar.parse_table.at(cur_symbol).at(next_token_symbol);
                vector<SymbolId> production = grammar.productions.at(production_number).production_rule;

                symbol_stack.pop();

                // Add to AST stack
    
                symbol_stack.push(production_number);

                for (int j = production.size() - 1; j >= 0; j--) {
                    symbol_stack.push(production.at(j));
                }

            }
        } else {
            // Pop from AST stack and push a new AST node based on the rule
            bool added = construct_production_node(get<int>(symbol_stack.top()), ast_stack);

            symbol_stack.pop();
        }


    }

    root = ast_stack.top();

}

// Given the root of a expression tree, find the infix notation of the tree, and store it in the infix parameter

/**
 * @brief Given the root of a expression tree, find its infix notation and place it in infix
 * 
 * @param node - Root of an expression tree
 * @param infix - Vector to store nodes in infix order (should be empty)
 */
void AST::get_infix(ID::ASTNodeId node, vector<ID::ASTNodeId>& infix) const {
    // There are only binary operations for now so each node in an expression tree that is not a leaf node should have two children

    uint32_t num_children = get_node(node)->children.size();

    assert(num_children == 0 || num_children == 2);

    if (num_children == 2) {
        get_infix(get_node(node)->children.at(0), infix);
    }

    infix.push_back(node);

    if (num_children == 2) {
        get_infix(get_node(node)->children.at(1), infix);
    }
}

/**
 * @brief Convert an infix expression to a postfix expression
 * 
 * @param infix - Infix expression
 * @param postfix - Vector to place postfix expression into (should be empty)
 * 
 * See https://www.geeksforgeeks.org/dsa/convert-infix-expression-to-postfix-expression/ 
 * for algorithm details
 */
void AST::infix_to_postfix(const vector<ID::ASTNodeId>& infix, vector<ID::ASTNodeId>& postfix) const {
    std::stack<BINARY_OP> op_stack;

    // The nodes corresponding to the operators in op_stack are stored here
    std::stack<ID::ASTNodeId> node_stack; 

    for (ID::ASTNodeId node : infix) {
        if (get_node(node)->node_type == AST_NODE_TYPE::BINARY_OP_NODE) {
            BINARY_OP node_op = dynamic_cast<ASTBinaryOpNode*>(get_node(node).get())->op;
            OP_ASSOCIATIVITY_TYPE assoc_type = op_associativity_map.at(node_op);

            // If the new operator has a higher precedence than the one at the top of the stack,
            // then we always push it to the stack
            if (op_stack.empty() || (op_precedence_map.at(op_stack.top()) > op_precedence_map.at(node_op))) {
                op_stack.push(node_op);
                node_stack.push(node);
            } else if (op_precedence_map.at(op_stack.top()) == op_precedence_map.at(node_op) && assoc_type == OP_ASSOCIATIVITY_TYPE::RTL) {
                op_stack.push(node_op);
                node_stack.push(node);
            } else {
                while (!op_stack.empty()) {
                    postfix.push_back(node_stack.top());
                    op_stack.pop();
                    node_stack.pop();
                }

                node_stack.push(node);
                op_stack.push(node_op);
            }
        } else {
            postfix.push_back(node);
        }
    }

    while (!op_stack.empty()) {
        op_stack.pop();
        postfix.push_back(node_stack.top());
        node_stack.pop();
    }

}

/**
 * @brief Given the postfix for a tree, return the root of the new tree with operator precedence represented
 *        Traversing the new tree in postfix order will come up with the correct result
 * 
 * @param postfix - vector of nodes representing postfix notation of a expression tree
 * 
 * @return root node of the new expression tree
 */
ID::ASTNodeId AST::generate_tree_from_postfix(const vector<ID::ASTNodeId>& postfix) {
    stack<ID::ASTNodeId> node_stack;

    // Clean postfix nodes by removing children

    for (ID::ASTNodeId node : postfix) {
        get_node(node)->children.clear();
    }

    for (ID::ASTNodeId node : postfix) {
        if (get_node(node)->node_type == AST_NODE_TYPE::BINARY_OP_NODE) {
            ID::ASTNodeId rhs = node_stack.top();
            node_stack.pop();

            ID::ASTNodeId lhs = node_stack.top();
            node_stack.pop();

            get_node(node)->children.push_back(lhs);
            get_node(node)->children.push_back(rhs);

            node_stack.push(node);
        } else {
            node_stack.push(node);
        }
    }

    return node_stack.top();
}

void AST::construct_expression_trees() {
    construct_expression_trees_helper(root);
}

void AST::construct_expression_trees_helper(ID::ASTNodeId node) {
    if (get_node(node)->node_type == AST_NODE_TYPE::BINARY_OP_NODE) {
        // std::cout << "Before:" << std::endl;
        // print_AST();

        vector<ID::ASTNodeId> infix;
        get_infix(node, infix);

        // print_vector("Infix", infix);

        vector<ID::ASTNodeId> postfix;
        infix_to_postfix(infix, postfix);

        // print_vector("Postfix", postfix);

        ID::ASTNodeId node_parent = get_node(node)->parent;

        auto node_pos_it = std::find(get_node(node_parent)->children.begin(), get_node(node_parent)->children.end(), node);

        remove_node(node);

        ID::ASTNodeId new_root = generate_tree_from_postfix(postfix);

        get_node(node_parent)->children.insert(node_pos_it, new_root);
    } else {
        for (ID::ASTNodeId child : get_node(node)->children) {
            construct_expression_trees_helper(child);
        }
    }
}