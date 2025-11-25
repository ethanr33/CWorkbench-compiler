
#pragma once

#include <unordered_map>
#include <memory>

#include "Slot.h"

enum class REGISTER_STATUS { FREE, IN_USE };

// Manages the storing of values in registers / stack
class SlotAllocator {
    private:
        std::unordered_map<REGISTER, REGISTER_STATUS> register_statuses = {
            {REGISTER::R8, REGISTER_STATUS::FREE},
            {REGISTER::R9, REGISTER_STATUS::FREE},
            {REGISTER::R10, REGISTER_STATUS::FREE},
            {REGISTER::R11, REGISTER_STATUS::FREE},
            {REGISTER::R12, REGISTER_STATUS::FREE},
            {REGISTER::R13, REGISTER_STATUS::FREE},
            {REGISTER::R14, REGISTER_STATUS::FREE},
        };

        std::unordered_map<ID::SymbolTableId, ID::SlotId> symbol_slots;

        Arena<std::unique_ptr<Slot>> slots;

        // Register set aside for address to address transfer
        const ID::SlotId temp_intermediate_slot_id = slots.add(std::make_unique<RegisterSlot>(REGISTER::R15));

        // Next available location to store in stack memory
        uint32_t next_available_stack_pos;

        ID::SlotId add_temp_to_stack(int size);
    public:
        SlotAllocator() : next_available_stack_pos(0) {}

        // Returns the index of a free slot to store a temporary value in
        // Prioritizes using free registers first, then stack memory
        ID::SlotId add_temporary(int size);

        // Returns the index of a free slot to store a value in
        // Always adds to stack memory (used for variables)
        ID::SlotId add_variable_to_stack(ID::SymbolTableId, int size);

        // Returns assembly code to access the value
        // For registers this is the name of the register
        // For stack values this is the string "[rbp-offset]" where offset is the value's stack offset
        std::string get_access_string(ID::SlotId) const;

        // Returns assembly code to set the value of this slot to a binary value
        // For registers this is mov "reg_name", val
        // For stack values this is mov [rbp-offset], val
        std::string get_set_val_instr(ID::SlotId, uint64_t new_val) const;

        // Returns assembly code to set the value of this slot to the value in another slot
        std::string get_set_val_instr_slot(ID::SlotId, ID::SlotId new_val);

        // Checks if a symbol already has memory allocated for it
        bool symbol_has_slot(ID::SymbolTableId) const;

        // Returns the slot where the symbol is stored
        // If symbol is not stored, return -1
        ID::SlotId get_symbol_slot(ID::SymbolTableId) const;
};
        