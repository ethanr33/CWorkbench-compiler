
#pragma once

#include <string>
#include <unordered_map>

#include "SlotAllocator.h"
#include "../tools/memory.h"

struct SymbolTableEntry {
    std::string identifier;
    ID::ASTNodeId node_id;
    ID::SymbolTableId table_id;

    SymbolTableEntry(const std::string& identifier, ID::ASTNodeId node_id) : identifier(identifier), node_id(node_id) {}
};

class SymbolTable {
    private:
        Arena<SymbolTableEntry> table_arena;
        std::unordered_map<std::string, ID::SymbolTableId> ident_to_entry;
        std::unordered_map<ID::ASTNodeId, ID::SymbolTableId> node_id_to_entry;
    public:
        static constexpr ID::SymbolTableId invalid_entry = -1;

        // size: specifies size of variable in bytes
        ID::SymbolTableId add_variable(ID::ASTNodeId, const std::string& string);

        bool has_identifier(const std::string&) const;

        SymbolTableEntry& get_by_identifier(const std::string&);
        const SymbolTableEntry& get_by_identifier(const std::string&) const;

        SymbolTableEntry& get_by_node_id(ID::ASTNodeId);
        const SymbolTableEntry& get_by_node_id(const ID::ASTNodeId) const;

        SymbolTableEntry& get_by_table_id(ID::SymbolTableId);
        const SymbolTableEntry& get_by_table_id(const ID::SymbolTableId) const;

};