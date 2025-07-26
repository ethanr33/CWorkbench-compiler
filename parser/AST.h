
#pragma once

#include <string>
#include <vector>
#include <stack>

#include "../tools/debug.h"
#include "../lexer/Token.h"
#include "CFG.h"

using std::string;
using std::vector;
using std::stack;

enum class AST_NODE_TYPE {
    ROOT_NODE,
    FUNCTION_NODE,
    RETURN_NODE,
    TYPE_NODE,
    IDENT_NODE,
    ARITHMETIC_EXPR_NODE,
    INT_CONST_NODE,
    TEMP_NODE
};

struct ASTNode {
    AST_NODE_TYPE node_type;
    vector<ASTNode*> children;
    ASTNode* parent;

    ASTNode(AST_NODE_TYPE type) : node_type(type), children({}) {}

    // You are responsible for deleting all ASTNode children before deleting the node itself!
    virtual ~ASTNode() {
        children.clear();
    }
};

struct ASTArithmeticExprNode : ASTNode {
    string op;

    ASTArithmeticExprNode() : ASTNode(AST_NODE_TYPE::ARITHMETIC_EXPR_NODE) {}
    ASTArithmeticExprNode(string op) : ASTNode(AST_NODE_TYPE::ARITHMETIC_EXPR_NODE), op(op) {}
};


struct ASTTempNode : ASTNode {
    string data;
    TOKEN_TYPE corresponding_token;
    ASTTempNode(string data, TOKEN_TYPE corresponding_token) : ASTNode(AST_NODE_TYPE::TEMP_NODE), corresponding_token(corresponding_token), data(data) {}
};

struct ASTRootNode : ASTNode {
    ASTRootNode() : ASTNode(AST_NODE_TYPE::ROOT_NODE) {}
};

struct ASTFunctionNode : ASTNode {
    string function_name;

    ASTFunctionNode() : ASTNode(AST_NODE_TYPE::FUNCTION_NODE) {}
    ASTFunctionNode(string function_name) : ASTNode(AST_NODE_TYPE::FUNCTION_NODE), function_name(function_name) {}
};

struct ASTReturnNode : ASTNode {
    ASTReturnNode() : ASTNode(AST_NODE_TYPE::RETURN_NODE) {}
};

struct ASTTypeNode : ASTNode {
    string type_name;
    ASTTypeNode() : ASTNode(AST_NODE_TYPE::TYPE_NODE) {}
    ASTTypeNode(string type_name) : ASTNode(AST_NODE_TYPE::TYPE_NODE), type_name(type_name) {}
};

struct ASTIntConstNode : ASTNode {
    int value;

    ASTIntConstNode(int value) : ASTNode(AST_NODE_TYPE::INT_CONST_NODE), value(value) {}
};

struct ASTIdentNode : ASTNode {
    string identifier;

    ASTIdentNode(const string& identifier) : ASTNode(AST_NODE_TYPE::IDENT_NODE), identifier(identifier) {}
};

template<typename T>
T* find_single_AST_node(ASTNode* parent, AST_NODE_TYPE to_find) {
    for (ASTNode* child : parent->children) {
        if (child->node_type == to_find) {
            return dynamic_cast<T*>(child);
        }
    }

    return nullptr;
}

struct AST {
    CFG grammar;
    ASTNode* root;

    AST() : root(nullptr) {}
    AST(CFG grammar) : root(nullptr), grammar(grammar) {}

    void clean_arithmetic_expr_tree(ASTNode*);

    void construct_parse_tree(const vector<Token>&);
    void construct_AST_from_parse_tree();
    void construct_production_node(int, stack<ASTNode*>&);
    void construct_leaf_node(Token&, Symbol*, stack<ASTNode*>&);
};