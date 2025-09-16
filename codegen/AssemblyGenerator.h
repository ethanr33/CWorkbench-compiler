
#include <string>

#include "SymbolTable.h"
#include "AssemblyBuilder.h"
#include "../parser/AST.h"

using std::string;

class AssemblyGenerator {
    private:
        // Variable to store the generated assembly code from convert_AST_to_assembly
        string generated_assembly;

        AST& ast;
        SymbolTable& symbol_table;

        AssemblyBuilder builder;

        string AST_to_assembly_helper(ID::ASTNodeId);
    public:

        AssemblyGenerator(AST& ast, SymbolTable& table) : generated_assembly(""), ast(ast), symbol_table(table), builder(AssemblyBuilder(ast, table)) {}

        // Given the AST which represents a program, write corresponding assembly code to generated_assembly
        void convert_AST_to_assembly();
        // Writes the contents of generated_assembly to a file with the name file_name
        void output_assembly_to_file(const string&) const;
};