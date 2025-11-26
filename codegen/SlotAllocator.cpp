#include "SlotAllocator.h"
#include <string>

ID::SlotId SlotAllocator::add_temp_to_stack(int size) {
    ID::SlotId assigned_slot = slots.add(std::make_unique<TempStackSlot>(size, next_available_stack_pos));
    next_available_stack_pos += size;
    return assigned_slot;
}

ID::SlotId SlotAllocator::add_temporary(int size) {
    // Check if there are any free registers to use
    
    bool has_free_register = false;
    REGISTER chosen_register;

    for (const auto&[reg, reg_status] : register_statuses) {
        if (reg_status == REGISTER_STATUS::FREE) {
            has_free_register = true;
            chosen_register = reg;
            break;
        }
    }

    ID::SlotId assigned_slot;

    // Register cannot fit value with size greater than 8 bytes
    if (has_free_register && size <= 8) {
        assigned_slot = slots.add(std::make_unique<RegisterSlot>(chosen_register));
        register_statuses.at(chosen_register) = REGISTER_STATUS::IN_USE;
    } else {
        // If there is no free register, use a stack location
        assigned_slot = add_temp_to_stack(size);
    }
    
    return assigned_slot;
}
	

ID::SlotId SlotAllocator::add_variable_to_stack(ID::SymbolTableId id, int size) {
    ID::SlotId slot_id = slots.add(std::make_unique<SymbolStackSlot>(id, size, next_available_stack_pos));
    next_available_stack_pos += size;

    // Keep track of ident in symbol table
    symbol_slots[id] = slot_id;

    return slot_id;
}

std::string SlotAllocator::get_access_string(ID::SlotId id) const {
	return slots.get(id)->get_access_string();
}

std::string SlotAllocator::get_set_val_instr(ID::SlotId id, uint64_t new_val) const {
	return slots.get(id)->get_set_val_instr(new_val);
}

std::string SlotAllocator::get_set_val_instr_slot(ID::SlotId id, ID::SlotId new_val) {
    // Since there's no memory to memory addressing mode in x86, we need to use a temporary register to store one memory value, then transfer it into another
    if (slots.get(new_val)->get_location_type() == LOCATION_TYPE::STACK && slots.get(id)->get_location_type() == LOCATION_TYPE::STACK) {
        int source_size = slots.get(new_val)->get_size();

        // Move new_val value into temporary storage register, then move temp storage register into dest

        const std::string temp_register_name = slots.get(temp_intermediate_slot_id)->get_access_string();
        const std::string new_val_access_str = slots.get(new_val)->get_access_string();
        const std::string dest_access_str = slots.get(id)->get_access_string();

        std::string new_val_to_temp_instr = std::format("mov {}, {}\n", temp_register_name, new_val_access_str);
        std::string temp_to_dest_instr = std::format("mov {}, {}", dest_access_str, temp_register_name);

        return new_val_to_temp_instr + temp_to_dest_instr;
    } else {
        // However, reg to reg, reg to mem and mem to reg addressing modes do exist

        std::string dest_string = slots.get(id)->get_access_string();
        std::string src_string = slots.get(new_val)->get_access_string();

        return std::format("mov {}, {}", dest_string, src_string);
    }
}

bool SlotAllocator::symbol_has_slot(ID::SymbolTableId symbol_id) const {
    return symbol_slots.find(symbol_id) != symbol_slots.end();
}

ID::SlotId SlotAllocator::get_symbol_slot(ID::SymbolTableId symbol_id) const {
    if (!symbol_has_slot(symbol_id)) {
        return -1;
    }

    return symbol_slots.at(symbol_id);
}