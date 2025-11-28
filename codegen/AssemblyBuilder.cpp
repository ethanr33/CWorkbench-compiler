
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

    ID::SlotId return_val = operand_stack.top();
    std::string return_val_access_str = allocator.get_access_string(return_val);

    // Return value of function is always RAX so it's fine if we force overwrite it here
    generated_assembly_epilog += std::format("mov rax, {}\n", return_val_access_str);

}

void AssemblyBuilder::visit(ASTTypeNode& node) {
    clear_generated_assembly();
}

void AssemblyBuilder::visit(ASTBinaryOpNode& node) {
    clear_generated_assembly();

    assert(node.op != BINARY_OP::INVALID);

    // Fetch operands

    ID::SlotId rhs_slot = operand_stack.top();
    std::string rhs_slot_identifier = allocator.get_access_string(rhs_slot);
    operand_stack.pop();

    ID::SlotId lhs_slot = operand_stack.top(); 
    std::string lhs_slot_identifier = allocator.get_access_string(lhs_slot);
    operand_stack.pop();

    if (node.op == BINARY_OP::ASSIGNMENT) {
            // Assignment is a special binary op case, because we need to get the identifier to assign to

            std::string assignment_instr = allocator.get_set_val_instr_slot(lhs_slot, rhs_slot);

            generated_assembly_epilog += std::format("{}\n", assignment_instr);
    } else {
        // Result of binary ops on two ints is also an int of size 4
        ID::SlotId result_slot = allocator.add_temporary(8);
        std::string result_slot_identifier = allocator.get_access_string(result_slot);

        std::string result_initialization_instr;

        if (node.op == BINARY_OP::ADDITION) {
            result_initialization_instr = allocator.get_set_val_instr(result_slot, 0);
        } else if (node.op == BINARY_OP::MULTIPLICATION) {
            result_initialization_instr = allocator.get_set_val_instr(result_slot, 1);
        } 

        std::string operator_instr = op_to_assembly_map.at(node.op);
        std::string lhs_instruction = allocator.generate_instr_from_slots(operator_instr, result_slot, lhs_slot);
        std::string rhs_instruction = allocator.generate_instr_from_slots(operator_instr, result_slot, rhs_slot);


        generated_assembly_epilog += std::format("{}\n", result_initialization_instr);
        generated_assembly_epilog += std::format("{}\n", lhs_instruction);
        generated_assembly_epilog += std::format("{}\n", rhs_instruction) + "\n";

        operand_stack.push(result_slot);
    }

}

void AssemblyBuilder::visit(ASTTempNode& node) {
    throw std::runtime_error("Found temp node in AST");
}

void AssemblyBuilder::visit(ASTIdentNode& node) {
    clear_generated_assembly();

    AST_NODE_TYPE parent_node_type = ast.get_node(node.parent)->node_type;

    // Functions do not use any memory (yet)
    if (parent_node_type == AST_NODE_TYPE::FUNCTION_NODE) {
        return;
    }

    ID::SymbolTableId ident_symbol = symbol_table.get_by_identifier(node.identifier).table_id;
    ID::SlotId temp_slot_id;

    if (allocator.symbol_has_slot(ident_symbol)) {
        temp_slot_id = allocator.get_symbol_slot(ident_symbol);
    } else {
        temp_slot_id = allocator.add_variable_to_stack(ident_symbol, 8);
    }

    operand_stack.push(temp_slot_id);

}

void AssemblyBuilder::visit(ASTIntConstNode& node) {
    clear_generated_assembly();

    ID::SlotId temp_slot_id = allocator.add_temporary(8);
    std::string set_val_instr = allocator.get_set_val_instr(temp_slot_id, node.value);

    operand_stack.push(temp_slot_id);

    generated_assembly_body += std::format("{}\n", set_val_instr);
}

void AssemblyBuilder::visit(ASTTempParentNode& node) {
    throw std::runtime_error("Found ASTTempParent node in trimmed AST");
}

void AssemblyBuilder::visit(ASTVariableDeclNode& node) {
    clear_generated_assembly();

    if (node.defines_here) {
        ID::SlotId value_to_set = operand_stack.top();
        operand_stack.pop();

        ID::SlotId variable_slot = operand_stack.top();
        operand_stack.pop();

        generated_assembly_prolog += std::format("push 0\n");

        std::string set_variable_instr = allocator.get_set_val_instr_slot(variable_slot, value_to_set);

        generated_assembly_epilog += std::format("{}\n", set_variable_instr);
    }
}
