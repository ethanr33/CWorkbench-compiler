
#pragma once

#include <vector>
#include <unordered_map>

#include "../lexer/Token.h"
#include "../tools/memory.h"

using std::vector;
using std::unordered_map;

enum class AST_NODE_TYPE {
    ROOT_NODE,
    FUNCTION_NODE,
    RETURN_NODE,
    TYPE_NODE,
    IDENT_NODE,
    BINARY_OP_NODE,
    INT_CONST_NODE,
    VARIABLE_DECL_NODE,
    TEMP_NODE,
    TEMP_PARENT_NODE,
};

class NodeVisitor;

enum class BINARY_OP {
    INVALID,
    ADDITION,
    ASSIGNMENT
};

static const unordered_map<std::string, BINARY_OP> binary_op_map = {
    {"+", BINARY_OP::ADDITION},
    {"=", BINARY_OP::ASSIGNMENT}
};

struct ASTNode {
    AST_NODE_TYPE node_type;
    vector<ID::ASTNodeId> children;
    ID::ASTNodeId id;
    ID::ASTNodeId parent;

    virtual void visit(NodeVisitor&) = 0;

    ASTNode(AST_NODE_TYPE type) : node_type(type), children({}), parent(Arena<ASTNode>::invalid_id) {}
};

struct ASTBinaryOpNode : ASTNode {
    BINARY_OP op;

    ASTBinaryOpNode() : ASTNode(AST_NODE_TYPE::BINARY_OP_NODE), op(BINARY_OP::INVALID) {}

    void visit(NodeVisitor& visitor) override;

    void set_binary_op(const string& op_string) {
        op = binary_op_map.at(op_string);
    }
};


struct ASTTempNode : ASTNode {
    string data;
    TOKEN_TYPE corresponding_token;
    ASTTempNode(string data, TOKEN_TYPE corresponding_token) : ASTNode(AST_NODE_TYPE::TEMP_NODE), corresponding_token(corresponding_token), data(data) {}
    void visit(NodeVisitor& visitor) override;
};

struct ASTTempParentNode : ASTNode {
    string symbol;

    ASTTempParentNode(string symbol) : ASTNode(AST_NODE_TYPE::TEMP_PARENT_NODE), symbol(symbol) {}
    void visit(NodeVisitor& visitor) override;
};

struct ASTRootNode : ASTNode {
    ASTRootNode() : ASTNode(AST_NODE_TYPE::ROOT_NODE) {}
    void visit(NodeVisitor& visitor) override;
};

struct ASTFunctionNode : ASTNode {
    ASTFunctionNode() : ASTNode(AST_NODE_TYPE::FUNCTION_NODE) {}
    void visit(NodeVisitor& visitor) override;
};

struct ASTVariableDeclNode : ASTNode {
    bool defines_here;

    ASTVariableDeclNode() : ASTNode(AST_NODE_TYPE::VARIABLE_DECL_NODE) {}

    void visit(NodeVisitor& visitor) override;
};

struct ASTReturnNode : ASTNode {
    ASTReturnNode() : ASTNode(AST_NODE_TYPE::RETURN_NODE) {}
    void visit(NodeVisitor& visitor) override;
};

struct ASTTypeNode : ASTNode {
    string type_name;
    ASTTypeNode() : ASTNode(AST_NODE_TYPE::TYPE_NODE) {}
    ASTTypeNode(string type_name) : ASTNode(AST_NODE_TYPE::TYPE_NODE), type_name(type_name) {}

    void visit(NodeVisitor& visitor) override;
};

struct ASTIntConstNode : ASTNode {
    int value;

    ASTIntConstNode(int value) : ASTNode(AST_NODE_TYPE::INT_CONST_NODE), value(value) {}

    void visit(NodeVisitor& visitor) override;
};

struct ASTIdentNode : ASTNode {
    string identifier;

    ASTIdentNode(const string& identifier) : ASTNode(AST_NODE_TYPE::IDENT_NODE), identifier(identifier) {}

    void visit(NodeVisitor& visitor) override;
};