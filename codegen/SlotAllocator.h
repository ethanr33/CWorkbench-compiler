
#include <unordered_set>

#include "Slot.h"

// Manages the storing of values in registers / stack
class SlotAllocator {
    private:
        std::unordered_set<REGISTER> free_registers = {
            REGISTER::R8,
            REGISTER::R9,
            REGISTER::R10,
            REGISTER::R11,
            REGISTER::R12,
            REGISTER::R13,
            REGISTER::R14,
            REGISTER::R15
        };

        Arena<Slot> slots;
    public:
        // Returns the index of a free slot to store a value in
        // Prioritizes using free registers first, then stack memory
        ID::SlotId add();

        // Returns the index of a free slot to store a value in
        // Always adds to stack memory
        ID::SlotId add_to_stack();

        // Returns assembly code to access the value
        // For registers this is the name of the register
        // For stack values this is the string "[rbp-offset]" where offset is the value's stack offset
        std::string get_access_string(ID::SlotId) const;

        // Returns assembly code to set the value of this slot
        // For registers this is mov "reg_name", val
        // For stack values this is mov [rbp-offset], val
        std::string get_set_val_instr(ID::SlotId, uint64_t new_val) const;
};
        