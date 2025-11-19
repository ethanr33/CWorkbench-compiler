
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

    generated_assembly_epilog += std::format("mov rax, {}\n", allocator.get_access_string());

    if (std::get_if<int>(&operand_stack.top())) {
        generated_assembly_epilog = std::format("mov rax, {:d}\n", std::get<int>(operand_stack.top()));
    } else if (std::get_if<string>(&operand_stack.top())) {
        uint32_t offset = symbol_table.get_by_identifier(std::get<string>(operand_stack.top())).offset;
        generated_assembly_epilog += std::format("mov rax, [rbp-{:d}]\n", offset);  
    } else if (std::get_if<Register>(&operand_stack.top())) {
        Register reg = std::get<Register>(operand_stack.top());
        generated_assembly_epilog += std::format("mov rax, {}\n", register_keyword_map.at(reg));
    }
}

void AssemblyBuilder::visit(ASTTypeNode& node) {
    clear_generated_assembly();
}

void AssemblyBuilder::visit(ASTBinaryOpNode& node) {
    clear_generated_assembly();

    assert(node.op != BINARY_OP::INVALID);

    if (node.op == BINARY_OP::ASSIGNMENT) {
            // Assignment is a special binary op case, because we need to get the identifier to assign to

            ID::SlotId rhs = operand_stack.top();

            operand_stack.pop();

            // If we are assigning to anything but a identifier, then this is not allowed!
            if (!std::get_if<string>(&operand_stack.top())) {
                throw std::runtime_error("Attempted to assign to non-assignable value");
            }

            uint32_t lhs_offset = symbol_table.get_by_identifier(std::get<string>(operand_stack.top())).offset;

            if (std::get_if<int>(&rhs)) {
                generated_assembly_epilog += std::format("mov r14, {:d}\n", std::get<int>(rhs));
                generated_assembly_epilog += std::format("mov [rbp-{:d}], r14\n", lhs_offset);
            } else if (std::get_if<string>(&rhs)) {
                uint32_t rhs_offset = symbol_table.get_by_identifier(std::get<string>(rhs)).offset;
                generated_assembly_epilog += std::format("mov r14, [rbp-{:d}]\n", rhs_offset);
                generated_assembly_epilog += std::format("mov [rbp-{:d}], r14\n", lhs_offset);  
            } else if (std::get_if<Register>(&rhs)) {
                Register reg = std::get<Register>(rhs);
                generated_assembly_epilog += std::format("mov [rbp-{:d}], {}\n", lhs_offset, register_keyword_map.at(reg));
            }     

            operand_stack.pop();
    } else {
        if (std::get_if<int>(&operand_stack.top())) {
            generated_assembly_epilog += std::format("mov r12, {:d}\n", std::get<int>(operand_stack.top()));
        } else if (std::get_if<string>(&operand_stack.top())) {
            uint32_t offset = symbol_table.get_by_identifier(std::get<string>(operand_stack.top())).offset;
            generated_assembly_epilog += std::format("mov r12, [rbp-{:d}]\n", offset);  
        } else if (std::get_if<Register>(&operand_stack.top())) {
            Register reg = std::get<Register>(operand_stack.top());
            generated_assembly_epilog += std::format("mov r12, {}\n", register_keyword_map.at(reg));
        }     

        operand_stack.pop();
        
        if (std::get_if<int>(&operand_stack.top())) {
            generated_assembly_epilog += std::format("mov r13, {:d}\n", std::get<int>(operand_stack.top()));
        } else if (std::get_if<string>(&operand_stack.top())) {
            uint32_t offset = symbol_table.get_by_identifier(std::get<string>(operand_stack.top())).offset;
            generated_assembly_epilog += std::format("mov r13, [rbp-{:d}]\n", offset);  
        } else if (std::get_if<Register>(&operand_stack.top())) {
            Register reg = std::get<Register>(operand_stack.top());
            generated_assembly_epilog += std::format("mov r13, {}\n", register_keyword_map.at(reg));
        }    

        operand_stack.pop();

        string assembly_instruction = op_to_assembly_map.at(node.op);

        // Multiplication needs the initial value of rax to be 1, otherwise the result will always be 0
        if (node.op == BINARY_OP::MULTIPLICATION) {
            generated_assembly_epilog += "mov rax, 1\n";
        } else {
            generated_assembly_epilog += "mov rax, 0\n";
        }

        // If the operator is commutative, it doesn't really matter in what order the operands are combined together
        // However if the operator isn't commutative, we can't really specify an initial value for rax, and so rax should be set to the rhs of the operation (r13)
        if (!op_commutativity_map.at(node.op)) {
            // r13 is the second value popped and is the rhs of the operation, so we apply the operation r13 OP r12
            generated_assembly_epilog += std::format("mov rax, r13\n");
            generated_assembly_epilog += std::format("{} rax, r12\n", assembly_instruction);
        } else {
            generated_assembly_epilog += std::format("{} rax, r12\n", assembly_instruction);
            generated_assembly_epilog += std::format("{} rax, r13\n", assembly_instruction);
        }

        operand_stack.push(Register::RAX);  
    }

}

void AssemblyBuilder::visit(ASTTempNode& node) {
    throw std::runtime_error("Found temp node in AST");
}

void AssemblyBuilder::visit(ASTIdentNode& node) {
    clear_generated_assembly();

    AST_NODE_TYPE parent_type = ast.get_node(node.parent)->node_type;
    switch (parent_type) {
        case AST_NODE_TYPE::RETURN_NODE:
            operand_stack.push(node.identifier);
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

    // Ints are always 4 bytes
    ID::SlotId temp_slot_id = allocator.add_temporary(4);
    operand_stack.push(temp_slot_id);
}

void AssemblyBuilder::visit(ASTTempParentNode& node) {
    throw std::runtime_error("Found ASTTempParent node in trimmed AST");
}

void AssemblyBuilder::visit(ASTVariableDeclNode& node) {
    clear_generated_assembly();

    allocator.add_variable_to_stack(symbol_table.get_by_node_id(node.id).table_id, 4);
}
