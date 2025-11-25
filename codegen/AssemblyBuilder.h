
#include <variant>
#include <stack>

#include "SlotAllocator.h"
#include "../parser/ASTVisitors.h"

class AssemblyBuilder : public NodeVisitor {
    private:
        string generated_assembly_prolog;
        string generated_assembly_body;
        string generated_assembly_epilog;

        AST& ast;
        SymbolTable& symbol_table;

        SlotAllocator allocator;

        // Maps from binary operation to x86 assembly instruction for certain binary operations (arithmetic ones for now)
        // The operations here are meant to be used in two-operand form
        const std::unordered_map<BINARY_OP, string> op_to_assembly_map = {
            {BINARY_OP::ADDITION, "add"},
            {BINARY_OP::SUBTRACTION, "sub"},
            {BINARY_OP::MULTIPLICATION, "imul"}
        };

        bool has_valid_entry_point() const;
        void clear_generated_assembly();

        std::stack<ID::SlotId> operand_stack;
        
    public:

        AssemblyBuilder(AST& ast, SymbolTable& table) : ast(ast), symbol_table(table), allocator() {}

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

         ~AssemblyBuilder() = default;

};