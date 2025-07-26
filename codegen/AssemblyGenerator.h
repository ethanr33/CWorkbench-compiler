
#include <string>

#include "../parser/AST.h"

using std::string;

class AssemblyGenerator {
    private:
        // Variable to store the generated assembly code from convert_AST_to_assembly
        string generated_assembly;

        void generate_arithmetic_expr_assembly(ASTArithmeticExprNode* root, const string&, string&) const;
    public:
        // Given the AST which represents a program, write corresponding assembly code to generated_assembly
        void convert_AST_to_assembly(AST&);
        // Writes the contents of generated_assembly to a file with the name file_name
        void output_assembly_to_file(const string&) const;
};