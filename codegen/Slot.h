
#pragma once

#include <string>
#include <unordered_map>
#include <format>

#include "../tools/memory.h"
#include "SymbolTable.h"

enum class LOCATION_TYPE { REGISTER, STACK };
enum class REGISTER { RAX, R8, R9, R10, R11, R12, R13, R14, R15 };

// Class to represent a area in memory/registers where a value lives
class Slot {
    private:
        // Where is the value located (register, stack, etc.)
        LOCATION_TYPE location;
        // How big is the value (in bytes)
        int size;
    protected:
        // Map register type enum to its keyword in x86-64 asm
        const std::unordered_map<REGISTER, std::string> register_keyword_map = {
            {REGISTER::RAX, "rax"},
            {REGISTER::R8, "r8"},
            {REGISTER::R9, "r9"},
            {REGISTER::R10, "r10"},
            {REGISTER::R11, "r11"},
            {REGISTER::R12, "r12"},
            {REGISTER::R13, "r13"},
            {REGISTER::R14, "r14"},
            {REGISTER::R15, "r15"}
        };
    public:

        Slot(LOCATION_TYPE location, int size): location(location), size(size) {}

        LOCATION_TYPE get_location_type() const {
            return location;
        }

        int get_size() const {
            return size;
        }

        // Return x86 assembly code to access the value
        virtual std::string get_access_string() const = 0;

        // Returns x86 assembly code which sets the value in the slot to val
        virtual std::string get_set_val_instr(uint64_t) const = 0;

        // Returns x86 assembly which initializes this memory slot to the parameter
        virtual std::string get_initialization_instr(uint64_t) const = 0;
};

class RegisterSlot : public Slot {
    private:
        // The register this value is in
        REGISTER reg;
    public:
        // x86_64 registers are 8 bytes big
        RegisterSlot(REGISTER reg) : reg(reg), Slot(LOCATION_TYPE::REGISTER, 8) {}

        virtual std::string get_access_string() const override {
            return register_keyword_map.at(reg);
        };

        virtual std::string get_set_val_instr(uint64_t val) const override {
            std::string reg_name = get_access_string();

            return std::format("mov {}, {}", reg_name, val);
        }

        virtual std::string get_initialization_instr(uint64_t val) const override {
            return get_set_val_instr(val);
        }

};

// Avoid storing the offset directly in the class if necessary,
// This could lead to different sources of truth
class StackSlot : public Slot {
    protected:
        uint32_t offset;
    public:
        StackSlot(int size, uint32_t offset) : offset(offset), Slot(LOCATION_TYPE::STACK, size) {}

        virtual uint32_t get_offset() const = 0;

        virtual std::string get_access_string() const override {
            uint32_t offset = get_offset();
            return std::format("[rbp-{}]", offset);
        };

        virtual std::string get_set_val_instr(uint64_t val) const override {
            std::string access_str = get_access_string();
            return std::format("mov {}, {}", access_str, val);
        }

        virtual std::string get_initialization_instr(uint64_t val) const override {
            return std::format("push {}", val);
        }
};

// It's fine to store stack offset here because this is a temporary not stored in the symbol table
class TempStackSlot : public StackSlot {
    public:
        TempStackSlot(int size, uint32_t offset) : StackSlot(size, offset) {}

        virtual uint32_t get_offset() const override {
            return offset;
        }

};

class SymbolStackSlot : public StackSlot {
    private:
        ID::SymbolTableId id;
    public:
        SymbolStackSlot(ID::SymbolTableId id, int size, int offset) : id(id), StackSlot(size, offset) {}

        virtual uint32_t get_offset() const override {
            return offset;
        }
};
