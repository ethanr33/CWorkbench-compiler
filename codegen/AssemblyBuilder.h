
#include <variant>
#include <stack>

#include "../parser/ASTVisitors.h"

enum class Register { RAX, R12 };

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

        // Maps from binary operation to x86 assembly instruction for certain binary operations (arithmetic ones for now)
        // The operations here are meant to be used in two-operand form
        const std::unordered_map<BINARY_OP, string> op_to_assembly_map = {
            {BINARY_OP::ADDITION, "add"},
            {BINARY_OP::MULTIPLICATION, "imul"}
        };

        std::unordered_map<Register, string> register_keyword_map = {
            {Register::RAX, "rax"},
            {Register::R12, "r12"}
        };

        bool has_valid_entry_point() const;
        void clear_generated_assembly();

        void allocate_memory_helper(ID::ASTNodeId);

        std::stack<std::variant<int, std::string, Register>> operand_stack;
        
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