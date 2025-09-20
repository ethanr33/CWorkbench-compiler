
#include "../parser/ASTVisitors.h"


class AssemblyBuilder : public NodeVisitor {
    private:
        string generated_assembly_prolog;
        string generated_assembly_body;
        string generated_assembly_epilog;

        AST& ast;
        SymbolTable& symbol_table;

        bool has_valid_entry_point() const;
        void clear_generated_assembly();

        std::unordered_map<string, bool> used_registers = {
            {"r12", false},
            {"r13", false}
        };
        
    public:

        AssemblyBuilder(AST& ast, SymbolTable& table) : ast(ast), symbol_table(table) {}

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
        void initialize_local_variables();

         ~AssemblyBuilder() = default;

};