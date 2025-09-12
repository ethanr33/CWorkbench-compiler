
#include "ASTVisitors.h"

class AssemblyBuilder : NodeVisitor {
    private:
        string generated_assembly;
        AST& ast;
        SymbolTable& symbol_table;

        bool has_valid_entry_point() const;
        
    public:

        AssemblyBuilder(AST& ast, SymbolTable& table) : ast(ast), symbol_table(table) {}

        void visit(ASTRootNode&) override;
        void visit(ASTFunctionNode&) override;
        void visit(ASTReturnNode&) override;
        void visit(ASTTypeNode&) override;
        void visit(ASTBinaryOpNode&) override;
        void visit(ASTTempNode&) override;
        void visit(ASTIdentNode&) override;
        void visit(ASTIntConstNode&) override;

        void validate_AST() const;

         ~AssemblyBuilder() = default;

};