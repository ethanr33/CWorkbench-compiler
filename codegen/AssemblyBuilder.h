
#include <variant>
#include <stack>

#include "../parser/ASTVisitors.h"

using RegisterID = unsigned char;

class MemoryAllocator : public NodeVisitor {
    private:
        AST& ast;
        SymbolTable& symbol_table;

        uint32_t max_stack_offset = 0;
    public:

        MemoryAllocator(AST& ast, SymbolTable& table) : ast(ast), symbol_table(table) {}

        void visit(ASTRootNode&) override {};
        void visit(ASTFunctionNode&) override {};
        void visit(ASTReturnNode&) override {};
        void visit(ASTTypeNode&) override {};
        void visit(ASTBinaryOpNode&) override {};
        void visit(ASTTempNode&) override {};
        void visit(ASTIdentNode&) override {};
        void visit(ASTIntConstNode&) override {};
        void visit(ASTTempParentNode&) override {}
        void visit(ASTVariableDeclNode&) override;
};

class AssemblyBuilder : public NodeVisitor {
    private:
        string generated_assembly_prolog;
        string generated_assembly_body;
        string generated_assembly_epilog;

        AST& ast;
        SymbolTable& symbol_table;

        MemoryAllocator allocator;

        bool has_valid_entry_point() const;
        void clear_generated_assembly();

        void allocate_memory_helper(ID::ASTNodeId);

        std::stack<std::variant<int, std::string, RegisterID>> operand_stack;

        std::unordered_map<string, bool> used_registers = {
            {"r12", false},
            {"r13", false},
            {"r14", false}
        };
        
    public:

        AssemblyBuilder(AST& ast, SymbolTable& table) : ast(ast), symbol_table(table), allocator(ast, symbol_table) {}

        const string& get_prolog() const;
        const string& get_body() const;
        const string& get_epilog() const;

        void visit(ASTRootNode&) override;
        void visit(ASTFunctionNode&) override;
        void visit(ASTReturnNode&) override;
        void visit(ASTTypeNode&) override;
        void visit(ASTBinaryOpNode&) override;
        void visit(ASTTempNode&) override;
        void visit(ASTIdentNode&) override;
        void visit(ASTIntConstNode&) override;
        void visit(ASTTempParentNode&) override;
        void visit(ASTVariableDeclNode&) override;

        void validate_AST() const;
        void allocate_memory();

         ~AssemblyBuilder() = default;

};