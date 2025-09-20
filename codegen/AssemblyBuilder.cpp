
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
    generated_assembly_prolog += "[BITS 64]\nglobal _start\n\nsection .text\n";

    generated_assembly_epilog += "_start:\n";
    generated_assembly_epilog += "call main\n";
    generated_assembly_epilog += "mov rdi, rax\n";
    generated_assembly_epilog += "mov rax, 60\n";
    generated_assembly_epilog += "syscall";
}

void AssemblyBuilder::visit(ASTFunctionNode& node) {
    clear_generated_assembly();

    if (symbol_table.get_by_node_id(node.id).identifier == "main") {
        generated_assembly_prolog += "main:\n";
        generated_assembly_prolog += "push rbp\n";
        generated_assembly_prolog += "mov rbp, rsp\n";

        generated_assembly_epilog += "mov rsp, rbp\n";
        generated_assembly_epilog += "pop rbp\n";
        generated_assembly_epilog += "ret\n";
    } 
}

void AssemblyBuilder::visit(ASTReturnNode& node) {
    clear_generated_assembly();

}

void AssemblyBuilder::visit(ASTTypeNode& node) {
    clear_generated_assembly();
}

void AssemblyBuilder::visit(ASTBinaryOpNode& node) {
    clear_generated_assembly();

    generated_assembly_epilog += "add rax, r12\n";

    if (used_registers.at("r13")) {
        generated_assembly_epilog += "add rax, r13\n";
        used_registers.at("r13") = false;
    }

    used_registers.at("r12") = false;
}

void AssemblyBuilder::visit(ASTTempNode& node) {
    assert("Found temp node in AST");
}

void AssemblyBuilder::visit(ASTIdentNode& node) {
    clear_generated_assembly();

    int offset = symbol_table.get_by_identifier(node.identifier).offset;

    AST_NODE_TYPE parent_type = ast.get_node(node.parent)->node_type;
    switch (parent_type) {
        case AST_NODE_TYPE::RETURN_NODE:
            generated_assembly_body += std::format("mov rax, [ebp-{:d}]\n", offset);
            break;
        case AST_NODE_TYPE::BINARY_OP_NODE:
            if (!used_registers.at("r12")) {
                generated_assembly_body += std::format("mov rax, [ebp-{:d}]\n", offset);
                used_registers.at("r12") = true;
            } else if (!used_registers.at("r13")) {
                generated_assembly_body += std::format("mov rax, [ebp-{:d}]\n", offset);
                used_registers.at("r13") = true;
            }
            break;
        case AST_NODE_TYPE::VARIABLE_DECL_NODE:
            if (!used_registers.at("r12")) {
                generated_assembly_body += std::format("mov rax, [ebp-{:d}]\n", offset);
                used_registers.at("r12") = true;
            } else if (!used_registers.at("r13")) {
                generated_assembly_body += std::format("mov rax, [ebp-{:d}]\n", offset);
                used_registers.at("r13") = true;
            }
            break;
        default:
            throw std::runtime_error(std::format("Invalid node arrangement: INT_CONST node is a child of {:d}",  std::to_underlying(parent_type)));
            break;
    }
}

void AssemblyBuilder::visit(ASTIntConstNode& node) {
    clear_generated_assembly();

    AST_NODE_TYPE parent_type = ast.get_node(node.parent)->node_type;
    switch (parent_type) {
        case AST_NODE_TYPE::RETURN_NODE:
            generated_assembly_body += std::format("mov rax, {:d}\n", node.value);
            break;
        case AST_NODE_TYPE::BINARY_OP_NODE:
            if (!used_registers.at("r12")) {
                generated_assembly_body += std::format("mov r12, {:d}\n", node.value);
                used_registers.at("r12") = true;
            } else if (!used_registers.at("r13")) {
                generated_assembly_body += std::format("mov r13, {:d}\n", node.value);
                used_registers.at("r13") = true;
            }
            break;
        case AST_NODE_TYPE::VARIABLE_DECL_NODE:
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

void AssemblyBuilder::visit(ASTTempParentNode& node) {
    assert("Found temp parent node in AST");
}

void AssemblyBuilder::visit(ASTVariableDeclNode& node) {
    clear_generated_assembly();

    generated_assembly_epilog += "push r12\n";
    symbol_table.get_by_node_id(node.id).offset = variable_stack_offset;
    variable_stack_offset += 4;

    used_registers.at("r12") = false;
}