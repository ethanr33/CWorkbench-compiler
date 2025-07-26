
#include <fstream>
#include <stdexcept>

#include "AssemblyGenerator.h"

using std::ofstream;
using std::to_string;

/*
    Currently, the compiler is only capable of compiling a program with a simple int main() function with no parameters that returns an integer
    When more functionality is 
*/

void AssemblyGenerator::generate_arithmetic_expr_assembly(ASTArithmeticExprNode* root, const string& register_name, string& output) const {
    
    bool all_const = true;

    for (ASTNode* child : root->children) {
        if (child->node_type == AST_NODE_TYPE::ARITHMETIC_EXPR_NODE) {
            all_const = false;
            generate_arithmetic_expr_assembly(dynamic_cast<ASTArithmeticExprNode*>(child), register_name, output);
        }
    }

    if (all_const) {
        ASTIntConstNode* child1 = dynamic_cast<ASTIntConstNode*>(root->children.at(0));
        ASTIntConstNode* child2 = dynamic_cast<ASTIntConstNode*>(root->children.at(1));

        output = "mov rdi, " + to_string(child1->value) + "\n" + "add rdi, " + to_string(child2->value) + "\n";
    } else {
        ASTIntConstNode* int_node = find_single_AST_node<ASTIntConstNode>(root, AST_NODE_TYPE::INT_CONST_NODE);
        output += "add rdi, " + to_string(int_node->value) + "\n";
    }

}

void AssemblyGenerator::convert_AST_to_assembly(AST& ast) {

    ASTNode* root = ast.root;

    // Locate our entry point, int main()

    ASTFunctionNode* entry_point = nullptr;

    for (ASTNode* child : root->children) {
        if (child->node_type == AST_NODE_TYPE::FUNCTION_NODE) {
            ASTFunctionNode* potential_entry_point = dynamic_cast<ASTFunctionNode*>(child);

            if (potential_entry_point->function_name == "main") {
                entry_point = potential_entry_point;
                break;
            }
        }
    }

    // If there is no entry point, then the program shouldn't compile (for now)
    if (entry_point == nullptr) {
        throw std::runtime_error("No entry point main() found");
    }

    // Get main()'s return value

    AST_NODE_TYPE return_val_type;
    ASTNode* return_val_root = nullptr;

    for (ASTNode* child : entry_point->children) {
        if (child->node_type == AST_NODE_TYPE::RETURN_NODE) {

            // Return nodes will only have one child, the return value
            ASTNode* return_val_node = child->children.at(0);
            return_val_type = return_val_node->node_type;

            if (return_val_type == AST_NODE_TYPE::ARITHMETIC_EXPR_NODE) {
                return_val_root = return_val_node;
            } else if (return_val_type == AST_NODE_TYPE::INT_CONST_NODE) {
                return_val_root = return_val_node;
            } else {
                throw std::runtime_error("main() should return an int");
            }
        }
    }

    if (return_val_root == nullptr) {
        throw std::runtime_error("No return statement found in main()");
    }

    // If main() is found and it has a valid return value, then this is a valid program, so generate the assembly

    // Define entry point
    generated_assembly = "[BITS 64]\nglobal _start\n_start:\n";
    
    // Add code for return value
    // Since we are putting code directly in _start, we need to use the exit syscall
    // if we use ret it will pop off an empty stack which would cause undefined behavior

    if (return_val_type == AST_NODE_TYPE::INT_CONST_NODE) {
        ASTIntConstNode* int_node = dynamic_cast<ASTIntConstNode*>(return_val_root);
        generated_assembly += "mov rax, 60\nmov rdi, " + std::to_string(int_node->value) + "\nsyscall";
    } else {
        ASTArithmeticExprNode* expr_node = dynamic_cast<ASTArithmeticExprNode*>(return_val_root);

        string calculations = "";
        generate_arithmetic_expr_assembly(expr_node, "rdi", calculations);
        generated_assembly += "mov rax, 60\n" + calculations + "\nsyscall";
    }


}

void AssemblyGenerator::output_assembly_to_file(const string& file_name) const {
    ofstream assembly_file(file_name);

    if (!assembly_file.is_open()) {
        throw std::runtime_error("Failed to open output file.");
    }

    assembly_file << generated_assembly;

    assembly_file.close();
}