
#include <format>
#include <assert.h>
#include <utility>

#include "../parser/AST.h"
#include "AssemblyBuilder.h"

void AssemblyBuilder::allocate_memory_helper(ID::ASTNodeId node_id) {

    ast.get_node(node_id)->visit(allocator);

    for (ID::ASTNodeId child : ast.get_node(node_id)->children) {
        allocate_memory_helper(child);
    }
} 

void AssemblyBuilder::allocate_memory() {
    allocate_memory_helper(ast.get_root_id());
}

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

    assert(node.op != BINARY_OP::INVALID);

    if (ast.get_node(node.parent)->node_type != AST_NODE_TYPE::BINARY_OP_NODE) {
        generated_assembly_prolog += "mov rax, 0\n";
    }

    switch (node.op) {
        case BINARY_OP::ADDITION:

            int num_operand_nodes;

            if (ast.get_node(node.children.at(1))->node_type == AST_NODE_TYPE::BINARY_OP_NODE) {
                num_operand_nodes = 1;
            } else {
                num_operand_nodes = 2;
            }

            if (std::get_if<int>(&operand_stack.top())) {
                generated_assembly_epilog += std::format("mov r12, {:d}\n", std::get<int>(operand_stack.top()));
                operand_stack.pop();
            } else {
                uint32_t offset = symbol_table.get_by_identifier(std::get<string>(operand_stack.top())).offset;
                generated_assembly_epilog += std::format("mov r12, [rbp - {:d}]\n", std::get<int>(operand_stack.top()));
                operand_stack.pop();
            }

            if (num_operand_nodes > 1 && std::get_if<int>(&operand_stack.top())) {
                generated_assembly_epilog += std::format("mov r13, {:d}\n", std::get<int>(operand_stack.top()));
                operand_stack.pop();
            } else if (num_operand_nodes > 1) {
                uint32_t offset = symbol_table.get_by_identifier(std::get<string>(operand_stack.top())).offset;
                generated_assembly_epilog += std::format("mov r13, [rbp - {:d}]\n", offset);
                operand_stack.pop();
            }

            generated_assembly_epilog += std::format("add rax, r12\n");

            if (num_operand_nodes > 1) {
                generated_assembly_epilog += std::format("add rax, r13\n");
            }
            
            break;
        case BINARY_OP::ASSIGNMENT:
            // Assignment is a special binary op case, because we need to get the identifier to assign to

            // if (std::get_if<int>(&operand_stack.top())) {
            //     generated_assembly_epilog += std::format("mov rax, {:d}", std::get<int>(operand_stack.top()));
            // } else {
            //     uint32_t offset = symbol_table.get_by_identifier(std::get<string>(operand_stack.top())).offset;
            //     generated_assembly_epilog += std::format("mov rax, [rbp - {:d}]\n", offset);
            // }

            operand_stack.pop();

            if (std::get_if<string>(&operand_stack.top())) {
                uint32_t offset = symbol_table.get_by_identifier(std::get<string>(operand_stack.top())).offset;
                generated_assembly_epilog += std::format("mov [rbp - {:d}], rax\n", offset);
            } else {
                throw std::runtime_error("LHS of assignment expression is not assignable");
            }

            operand_stack.pop();

            break;
    }
}

void AssemblyBuilder::visit(ASTTempNode& node) {
    assert("Found temp node in AST");
}

void AssemblyBuilder::visit(ASTIdentNode& node) {
    clear_generated_assembly();

    uint32_t offset = symbol_table.get_by_identifier(node.identifier).offset;

    AST_NODE_TYPE parent_type = ast.get_node(node.parent)->node_type;
    switch (parent_type) {
        case AST_NODE_TYPE::RETURN_NODE:
            generated_assembly_body += std::format("mov rax, [rbp - {:d}]\n", offset);
            break;
        case AST_NODE_TYPE::BINARY_OP_NODE:
            operand_stack.push(node.identifier);
            break;
        case AST_NODE_TYPE::VARIABLE_DECL_NODE:
            operand_stack.push(node.identifier);
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
            operand_stack.push(node.value);
            break;
        case AST_NODE_TYPE::VARIABLE_DECL_NODE:
            operand_stack.push(node.value);
            break;
        default:
            throw std::runtime_error(std::format("Invalid node arrangement: INT_CONST node is a child of {:d}",  std::to_underlying(parent_type)));
            break;
    }
}

void AssemblyBuilder::visit(ASTTempParentNode& node) {
    throw std::runtime_error("Found ASTTempParent node in trimmed AST");
}

void AssemblyBuilder::visit(ASTVariableDeclNode& node) {
    clear_generated_assembly();

    if (operand_stack.size() == 1) {
        generated_assembly_epilog += "push rax\n";
        return;
    }

    if (std::get_if<int>(&operand_stack.top())) {
        generated_assembly_epilog = std::format("push {:d}\n", std::get<int>(operand_stack.top()));
    } else {
        uint32_t offset = symbol_table.get_by_identifier(std::get<string>(operand_stack.top())).offset;
        generated_assembly_epilog += std::format("mov r14, [rbp - {:d}]\n", offset);  
        generated_assembly_epilog += std::format("push r14\n", offset);  
    }

    operand_stack.pop();
}

void MemoryAllocator::visit(ASTVariableDeclNode& node) {
    assert(ast.get_node(node.children.at(0))->node_type == AST_NODE_TYPE::IDENT_NODE);

    max_stack_offset += 8;
    symbol_table.get_by_node_id(node.id).offset = max_stack_offset;
}