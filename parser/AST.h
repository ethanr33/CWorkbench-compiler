
#pragma once

#include <string>
#include <vector>
#include <stack>

#include "../tools/debug.h"
#include "../lexer/Token.h"
#include "CFG.h"

using ASTNodeId = uint32_t;

using std::string;
using std::vector;
using std::stack;

class NodeVisitor;

enum class AST_NODE_TYPE {
    ROOT_NODE,
    FUNCTION_NODE,
    RETURN_NODE,
    TYPE_NODE,
    IDENT_NODE,
    ARITHMETIC_EXPR_NODE,
    BINARY_OP_NODE,
    INT_CONST_NODE,
    TEMP_NODE
};

enum class BINARY_OP {
    ADDITION
};

struct ASTNode {
    AST_NODE_TYPE node_type;
    vector<ASTNodeId> children;
    ASTNodeId parent;

    virtual void visit(NodeVisitor&) = 0;

    ASTNode(AST_NODE_TYPE type) : node_type(type), children({}), parent(Arena<ASTNode>::invalid_id) {}
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

    void visit(NodeVisitor& visitor) override {

    }
};

struct ASTFunctionNode : ASTNode {
    string function_name;

    ASTFunctionNode() : ASTNode(AST_NODE_TYPE::FUNCTION_NODE) {}
    ASTFunctionNode(string function_name) : ASTNode(AST_NODE_TYPE::FUNCTION_NODE), function_name(function_name) {}

    void visit(NodeVisitor& visitor) override {
        
    }
};

struct ASTReturnNode : ASTNode {
    ASTReturnNode() : ASTNode(AST_NODE_TYPE::RETURN_NODE) {}

    void visit(NodeVisitor& visitor) override {
        
    }
};

struct ASTTypeNode : ASTNode {
    string type_name;
    ASTTypeNode() : ASTNode(AST_NODE_TYPE::TYPE_NODE) {}
    ASTTypeNode(string type_name) : ASTNode(AST_NODE_TYPE::TYPE_NODE), type_name(type_name) {}

    void visit(NodeVisitor& visitor) override {
        
    }
};

struct ASTIntConstNode : ASTNode {
    int value;

    ASTIntConstNode(int value) : ASTNode(AST_NODE_TYPE::INT_CONST_NODE), value(value) {}

    void visit(NodeVisitor& visitor) override {
        
    }
};

struct ASTIdentNode : ASTNode {
    string identifier;

    ASTIdentNode(const string& identifier) : ASTNode(AST_NODE_TYPE::IDENT_NODE), identifier(identifier) {}

    void visit(NodeVisitor& visitor) override {
        
    }
};

class NodeVisitor {
    public:
        virtual void visit(ASTRootNode&) = 0;
        virtual void visit(ASTFunctionNode&) = 0;
        virtual void visit(ASTReturnNode&) = 0;
        virtual void visit(ASTTypeNode&) = 0;
        virtual void visit(ASTArithmeticExprNode&) = 0;
        virtual void visit(ASTTempNode&) = 0;
        virtual void visit(ASTIdentNode&) = 0;
};

template<typename T>
class ParameterizedNodeVisitor {
    public:
        virtual T visit(ASTRootNode&) = 0;
        virtual T visit(ASTFunctionNode&) = 0;
        virtual T visit(ASTReturnNode&) = 0;
        virtual T visit(ASTTypeNode&) = 0;
        virtual T visit(ASTArithmeticExprNode&) = 0;
        virtual T visit(ASTTempNode&) = 0;
        virtual T visit(ASTIdentNode&) = 0;
};

class ASTBuilderVisitor : public NodeVisitor {
    void visit(ASTRootNode&) override;
    void visit(ASTFunctionNode&) override;
    void visit(ASTReturnNode&) override;
    void visit(ASTTypeNode&) override;
    void visit(ASTArithmeticExprNode&) override;
    void visit(ASTTempNode&) override;
    void visit(ASTIdentNode&) override;
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
    ASTNodeId root;

    AST() : root(Arena<ASTNode>::invalid_id) {}
    AST(CFG grammar) : root(Arena<ASTNode>::invalid_id), grammar(grammar) {}

    void clean_arithmetic_expr_tree(ASTNodeId);

    void construct_parse_tree(const vector<Token>&);
    void construct_AST_from_parse_tree();
    void construct_production_node(int, stack<ASTNodeId>&);
    void construct_leaf_node(Token&, SymbolId, stack<ASTNodeId>&);

    void construct_AST_helper(ASTNodeId);
};