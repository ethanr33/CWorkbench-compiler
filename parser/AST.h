
#pragma once

#include <string>
#include <vector>
#include <stack>

#include "../tools/debug.h"
#include "../codegen/SymbolTable.h"
#include "CFG.h"
#include "ASTNodes.h"
#include "ASTVisitors.h"

using std::string;
using std::stack;

template<typename T>
T* find_single_AST_node(ASTNode* parent, AST_NODE_TYPE to_find) {
    for (ASTNode* child : parent->children) {
        if (child->node_type == to_find) {
            return dynamic_cast<T*>(child);
        }
    }

    return nullptr;
}

class AST {
    private:
        CFG& grammar;

        // Not owned by AST
        SymbolTable& symbol_table;

        ID::ASTNodeId root;

        Arena<std::unique_ptr<ASTNode>> node_arena;

        ASTBuilderVisitor ast_builder = ASTBuilderVisitor{*this};
        ASTPrinterVisitor ast_printer = ASTPrinterVisitor{*this};

        bool construct_production_node(int, stack<ID::ASTNodeId>&);
        void construct_leaf_node(Token&, SymbolId, stack<ID::ASTNodeId>&);

        void construct_AST_helper(ID::ASTNodeId);
        void print_AST_helper(ID::ASTNodeId, int level);

        void build_expression_trees();
        void get_infix(ID::ASTNodeId, vector<ID::ASTNodeId>&) const;

        ID::ASTNodeId add_node(std::unique_ptr<ASTNode>&&);

    public:
        AST(SymbolTable& table, CFG& grammar) : root(Arena<std::unique_ptr<ASTNode>>::invalid_id), symbol_table(table), grammar(grammar) {}

        std::unique_ptr<ASTNode>& get_node(ID::ASTNodeId);
        const std::unique_ptr<ASTNode>& get_node(ID::ASTNodeId) const;

        void remove_node(ID::ASTNodeId);
        void remove_node(ASTNode&);

        ID::ASTNodeId get_root_id();

        SymbolTable& get_symbol_table();
        const SymbolTable& get_symbol_table() const;

        void construct_parse_tree(const vector<Token>&);
        void construct_AST_from_parse_tree();

        void print_AST();

};


