
#include <stdexcept>
#include <iostream>
#include <variant>
#include <utility>
#include <algorithm>

#include "AST.h"

using std::stack;
using std::variant;
using std::get_if;
using std::get;
using std::runtime_error;

void print_AST_helper(ASTNode* root, int level) {
    std::cout << std::string(level, '-') << "Node: " << std::to_underlying(root->node_type) << std::endl;
    for (ASTNode* child : root->children) {
        print_AST_helper(child, level + 1);
    }
}

void AST::print_AST() {
    print_AST_helper(root, 0);
}

void get_statements(ASTNode* root, vector<ASTNode*>& statements) {

    if (root->node_type == AST_NODE_TYPE::RETURN_NODE || root->node_type == AST_NODE_TYPE::COPY_ASSIGNMENT_NODE) {
        statements.push_back(root);
    } else {
        for (ASTNode* child : root->children) {
            get_statements(child, statements);
        }
    }
}

void clean_arithmetic_expr_trees_helper(ASTArithmeticExprNode* root) {
    auto it = root->children.begin();

    while (it != root->children.end()) {
        ASTNode* child = *it;

        // Remove empty arithmetic expr nodes
        if (child->node_type == AST_NODE_TYPE::ARITHMETIC_EXPR_NODE && child->children.size() == 0) {
            it = root->children.erase(it, it + 1);
            delete child;
        } else {
            it++;
        }
    }

    ASTNode* child1 = root->children.at(0);
    ASTNode* child2;

    if (root->children.size() == 2) {
        child2 = root->children.at(1);
    }

    if (child1->node_type == AST_NODE_TYPE::ARITHMETIC_EXPR_NODE) {
        clean_arithmetic_expr_trees_helper(dynamic_cast<ASTArithmeticExprNode*>(child1));
    }

    if (root->children.size() == 2 && child2->node_type == AST_NODE_TYPE::ARITHMETIC_EXPR_NODE) {
        clean_arithmetic_expr_trees_helper(dynamic_cast<ASTArithmeticExprNode*>(child2));
    }

    // Propagate single children up
    if (root->children.size() == 1) {
        ASTNode* parent = root->parent;
        // Add child to parent's children, then delete current node
        parent->children.push_back(root->children.at(0));
        root->children.at(0)->parent = parent;

        it = parent->children.begin();
        
        // Remove current node's entry from parent's children list
        while (it != parent->children.end()) {
            if (*it == root) {
                it = parent->children.erase(it, it + 1);
                delete root;
                break;
            } else {
                it++;
            }
        }

        // Repeat for parent if it is also a arithmetic expression tree
        if (parent->node_type == AST_NODE_TYPE::ARITHMETIC_EXPR_NODE) {
            ASTArithmeticExprNode* parent2 = dynamic_cast<ASTArithmeticExprNode*>(parent);

            // Propagate operation when parent doesn't have one
            if (parent2->op == "" && root->op != "") {
                parent2->op = root->op;
            }

            clean_arithmetic_expr_trees_helper(parent2);
        }
    }

}

void AST::clean_arithmetic_expr_trees(ASTNode* root) {
    if (root->node_type == AST_NODE_TYPE::ARITHMETIC_EXPR_NODE) {
        clean_arithmetic_expr_trees_helper(dynamic_cast<ASTArithmeticExprNode*>(root));
    } else {
        for (ASTNode* child : root->children) {
            clean_arithmetic_expr_trees(child);
        }
    }
}

void AST::clean_function_body_trees(ASTNode* root) {
    if (root->node_type == AST_NODE_TYPE::FUNCTION_BODY_NODE) {
        vector<ASTNode*> statements;
        get_statements(root, statements);

        for (ASTNode* statement : statements) {
            statement->parent = root;
        }

        root->children.clear();
        root->children = statements;
    } else {
        for (ASTNode* child : root->children) {
            clean_function_body_trees(child);
        }
    }
}

void AST::clean_return_nodes(ASTNode* root) {
    if (root->node_type == AST_NODE_TYPE::RETURN_NODE) {
        ASTNode* child = root->children.at(0);
        ASTNode* grandchild = root->children.at(0)->children.at(0);
        grandchild->parent = root;
        
        root->children.clear();
        root->children.push_back(grandchild);

        delete child;
    } else {
        for (ASTNode* child : root->children) {
            clean_return_nodes(child);
        }
    }
}

void construct_AST_helper(ASTNode* root) {
    for (ASTNode* child : root->children) {
        construct_AST_helper(child);
    }

    if (root->node_type == AST_NODE_TYPE::TYPE_NODE) {
        ASTTypeNode* type_node = dynamic_cast<ASTTypeNode*>(root);
        ASTTempNode* temp = dynamic_cast<ASTTempNode*>(type_node->children.at(0));

        type_node->type_name = temp->data;
    } else if (root->node_type == AST_NODE_TYPE::ARITHMETIC_EXPR_NODE) {
        ASTArithmeticExprNode* expr_node = dynamic_cast<ASTArithmeticExprNode*>(root);

        for (ASTNode* child : root->children) {
            if (child->node_type == AST_NODE_TYPE::TEMP_NODE) {
                expr_node->op = dynamic_cast<ASTTempNode*>(child)->data;
            }
        }
    } else if (root->node_type == AST_NODE_TYPE::COPY_ASSIGNMENT_NODE) {
        ASTCopyAssignmentNode* assignment_node = dynamic_cast<ASTCopyAssignmentNode*>(root);
        ASTIdentNode* ident_node = find_single_AST_node<ASTIdentNode>(assignment_node, AST_NODE_TYPE::IDENT_NODE);
        assignment_node->identifier = ident_node->identifier;
    }

    auto it = root->children.begin();

    while (it != root->children.end()) {
        ASTNode* cur = *it;

        if (cur->node_type == AST_NODE_TYPE::TEMP_NODE || cur->node_type == AST_NODE_TYPE::TYPE_NODE) {
            it = root->children.erase(it, it + 1);
            delete cur;
        } else {
            it++;
        }

    }
}

void AST::construct_AST_from_parse_tree() {
    construct_AST_helper(root);
    clean_arithmetic_expr_trees(root);
    clean_function_body_trees(root);
    clean_return_nodes(root);
}

void AST::construct_leaf_node(Token& token, Symbol* symbol, stack<ASTNode*>& ast_stack) {
    switch (symbol->corresponding_token) {
        case TOKEN_TYPE::INT_CONST:
            ast_stack.push(new ASTIntConstNode(std::stoi(token.value)));
            break;
        case TOKEN_TYPE::IDENTIFIER:
            ast_stack.push(new ASTIdentNode(token.value));
            break;
        default:
            ast_stack.push(new ASTTempNode(symbol->symbol, symbol->corresponding_token));
            break;
    }
}

void AST::construct_production_node(int production_index, stack<ASTNode*>& ast_stack) {
    Rule rule = grammar.productions.at(production_index);
    string production_symbol = rule.symbol->symbol;

    ASTNode* parent_node;

    if (production_symbol == "<root>") {
        parent_node = new ASTRootNode();
        parent_node->parent = nullptr;
    } else if (production_symbol == "<function>") {
        parent_node = new ASTFunctionNode();
    } else if (production_symbol == "<return>" || production_symbol == "<returnval>") {
        parent_node = new ASTReturnNode();
    } else if (production_symbol == "<type>") {
        parent_node = new ASTTypeNode();
    } else if (production_symbol == "<arithmeticexpr>" || production_symbol == "<arithmeticexprsuffix>") {
        parent_node = new ASTArithmeticExprNode();
    } else if (production_symbol == "<funcbody>" || production_symbol == "<funcbodyprefix>") {
        parent_node = new ASTFunctionBodyNode();
    } else if (production_symbol == "<statement>") {
        parent_node = new ASTCopyAssignmentNode();
    } else {
        throw runtime_error("Attempted to create node for unknown production symbol " + production_symbol);
    }
    
    for (int i = 0; i < rule.production_rule.size(); i++) {
        if (rule.production_rule.at(i) == grammar.symbols.at("ε")) {
            continue;
        }

        ASTNode* child = ast_stack.top();
        child->parent = parent_node;

        parent_node->children.push_back(child);
        ast_stack.pop();
    }

    // Rules are applied and pushed to the stack in the order that they appear
    // When the stack is popped from, nodes are retrieved in reverse order
    // So the list of children needs to be reversed to preserve the original order of statements
    std::reverse(parent_node->children.begin(), parent_node->children.end());

    ast_stack.push(parent_node);
}

void AST::construct_parse_tree(const vector<Token>& token_stream) {
    stack<ASTNode*> ast_stack;
    stack<std::variant<Symbol*, int>> symbol_stack;

    int i = 0;

    symbol_stack.push(grammar.symbols.at("$"));
    symbol_stack.push(grammar.symbols.at("<root>"));

    while (true) {

        if (get_if<Symbol*>(&symbol_stack.top())) {
            Symbol* stack_top = *get_if<Symbol*>(&symbol_stack.top());

            if (stack_top->is_terminal) {
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
                if (TERMINAL_MAP.at(stack_top->symbol) == cur_token.token_type) {
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
                
                Symbol* cur_symbol = stack_top;
                Symbol* next_token_symbol = grammar.token_to_symbol(cur_token);

                if (grammar.parse_table.find(cur_symbol) == grammar.parse_table.end()) {
                    throw runtime_error("Syntax error");
                }

                if (grammar.parse_table.at(cur_symbol).find(next_token_symbol) == grammar.parse_table.at(cur_symbol).end()) {
                    throw runtime_error("Syntax error");
                }

                int production_number = grammar.parse_table.at(cur_symbol).at(next_token_symbol);
                vector<Symbol*> production = grammar.productions.at(production_number).production_rule;

                symbol_stack.pop();

                // Add to AST stack
    
                symbol_stack.push(production_number);

                for (int j = production.size() - 1; j >= 0; j--) {
                    symbol_stack.push(production.at(j));
                }

            }
        } else {
            // Pop from AST stack and push a new AST node based on the rule
            construct_production_node(get<int>(symbol_stack.top()), ast_stack);
            symbol_stack.pop();
        }


    }

    root = ast_stack.top();

}