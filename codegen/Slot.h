
#include <string>
#include <unordered_map>
#include <format>

#include "../tools/memory.h"

enum class LOCATION_TYPE { REGISTER, STACK };
enum class REGISTER { RAX, R8, R9, R10, R11, R12, R13, R14, R15 };

// Map register type enum to its keyword in x86-64 asm
std::unordered_map<REGISTER, std::string> register_keyword_map = {
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

// Class to represent a area in memory/registers where a value lives
class Slot {
    private:
        // Where is the value located (register, stack, etc.)
        LOCATION_TYPE location;
        // How big is the value (in bytes)
        int size;
    public:

        Slot(LOCATION_TYPE location, int size): location(location), size(size) {}

        // Return x86 assembly code to access the value
        virtual std::string get_access_string() const = 0;
        virtual std::string get_set_val_instr(int val) const = 0;
};

class RegisterSlot : public Slot {
    private:
        // The register this value is in
        REGISTER reg;
    public:
        RegisterSlot(REGISTER reg) : reg(reg), Slot(LOCATION_TYPE::REGISTER, 8) {}

        virtual std::string get_access_string() const override {
            return register_keyword_map.at(reg);
        };

        virtual std::string get_set_val_instr(int val) const override {
            std::string reg_name = get_access_string();
            return std::format("mov {}, {}", reg_name, val);
        }

};

class TempStackSlot : public Slot {
    private:

    public:
        TempStackSlot(ID::SymbolTableId id, int size) : Slot(LOCATION_TYPE::STACK, size) {}
        
};

