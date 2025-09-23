
#include <fstream>
#include <stdexcept>

#include "AssemblyGenerator.h"

using std::ofstream;
using std::to_string;

string AssemblyGenerator::AST_to_assembly_helper(ID::ASTNodeId node_id) {
    string result = "";

    for (ID::ASTNodeId child : ast.get_node(node_id)->children) {
        result += AST_to_assembly_helper(child);
    }

    ast.get_node(node_id)->visit(builder);

    if (ast.get_node(node_id)->children.size() == 0) {
        result = builder.get_body();
    }

    result = builder.get_prolog() + result +  builder.get_epilog();

    return result;    
}

void AssemblyGenerator::convert_AST_to_assembly() {
    builder.validate_AST();
    builder.allocate_memory();
    generated_assembly = AST_to_assembly_helper(ast.get_root_id());
}

void AssemblyGenerator::output_assembly_to_file(const string& file_name) const {
    ofstream assembly_file(file_name);

    if (!assembly_file.is_open()) {
        throw std::runtime_error("Failed to open output file.");
    }

    assembly_file << generated_assembly;

    assembly_file.close();
}