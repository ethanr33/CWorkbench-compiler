
#include <format>
#include <assert.h>
#include <utility>

#include "../parser/AST.h"
#include "AssemblyBuilder.h"

void AssemblyBuilder::clear_generated_assembly() {
    generated_assembly_prolog = "";
    generated_assembly_body = "";
    generated_assembly_epilog = "";
}

bool AssemblyBuilder::has_valid_entry_point() const {
    for (ID::ASTNodeId child : ast.get_node(ast.get_root_id())->children) {
        if (ast.get_node(child)->node_type == AST_NODE_TYPE::FUNCTION_NODE) {
            if (symbol_table.get_by_node_id(child).identifier == "main") {
                return true;
            }
        }
    }

    return false;
}

const string& AssemblyBuilder::get_prolog() const {
    return generated_assembly_prolog;
}

const string& AssemblyBuilder::get_body() const {
    return generated_assembly_body;
}

const string& AssemblyBuilder::get_epilog() const {
    return generated_assembly_epilog;
}

void AssemblyBuilder::validate_AST() const {
    if (!has_valid_entry_point()) {
        throw std::runtime_error("Program has no entry point main()");
    }
}

void AssemblyBuilder::visit(ASTRootNode& node) {
    clear_generated_assembly();
    generated_assembly_prolog += "[BITS 64]\nglobal _start\n";
}

void AssemblyBuilder::visit(ASTFunctionNode& node) {
    clear_generated_assembly();

    generated_assembly_prolog += "_start:\n";
}

void AssemblyBuilder::visit(ASTReturnNode& node) {
    clear_generated_assembly();
    generated_assembly_epilog += "mov rax, 60\n";
    generated_assembly_epilog += "syscall";
}

void AssemblyBuilder::visit(ASTTypeNode& node) {
    clear_generated_assembly();
}

void AssemblyBuilder::visit(ASTBinaryOpNode& node) {
    clear_generated_assembly();

    assert(!used_registers.at("r12"));

    generated_assembly_epilog += "add rdi, r12\n";

    if (used_registers.at("r13")) {
        generated_assembly_epilog += "add rdi, r13\n";
        used_registers.at("r13") = false;
    }

    used_registers.at("r12") = false;
}

void AssemblyBuilder::visit(ASTTempNode& node) {
    clear_generated_assembly();
}

void AssemblyBuilder::visit(ASTIdentNode& node) {
    clear_generated_assembly();
}

void AssemblyBuilder::visit(ASTIntConstNode& node) {
    clear_generated_assembly();

    AST_NODE_TYPE parent_type = ast.get_node(node.parent)->node_type;
    switch (parent_type) {
        case AST_NODE_TYPE::RETURN_NODE:
            generated_assembly_body += std::format("mov rdi, {:d}\n", node.value);
            break;
        case AST_NODE_TYPE::BINARY_OP_NODE:
            assert(used_registers.at("r12") && used_registers.at("r13"));

            if (!used_registers.at("r12")) {
                generated_assembly_body += std::format("mov r12, {:d}\n", node.value);
                used_registers.at("r12") = true;
            } else if (!used_registers.at("r13")) {
                generated_assembly_body += std::format("mov r13, {:d}\n", node.value);
                used_registers.at("r13") = true;
            }
            break;
        default:
            throw std::runtime_error(std::format("Invalid node arrangement: INT_CONST node is a child of {:d}",  std::to_underlying(parent_type)));
            break;
    }
}
