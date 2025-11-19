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
    } else {
        // If there is no free register, use a stack location
        assigned_slot = add_temp_to_stack(size);
    }
    
    return assigned_slot;
}
	

ID::SlotId SlotAllocator::add_variable_to_stack(ID::SymbolTableId id, int size) {
    ID::SlotId slot_id = slots.add(std::make_unique<SymbolStackSlot>(id, size));
    next_available_stack_pos += size;
    return slot_id;
}

std::string SlotAllocator::get_access_string(ID::SlotId id) const {
	return slots.get(id)->get_access_string();
}

std::string SlotAllocator::get_set_val_instr(ID::SlotId id, uint64_t new_val) const {
	return slots.get(id)->get_set_val_instr(new_val);
}