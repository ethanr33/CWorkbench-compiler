
#pragma once

#include <string>
#include <unordered_map>

#include "../tools/memory.h"

struct SymbolTableEntry {
    std::string identifier;
    ID::ASTNodeId node_id;

    SymbolTableEntry(const std::string& identifier, ID::ASTNodeId node_id) : identifier(identifier), node_id(node_id) {}
};

class SymbolTable {
    private:
        Arena<SymbolTableEntry> table_arena;
        std::unordered_map<std::string, ID::SymbolTableId> ident_to_entry;
        std::unordered_map<ID::ASTNodeId, ID::SymbolTableId> node_id_to_entry;
    public:

        static constexpr ID::SymbolTableId invalid_entry = -1;

        ID::SymbolTableId add(ID::ASTNodeId, const std::string& string);

        SymbolTableEntry& get(ID::SymbolTableId);
        const SymbolTableEntry& get(const ID::SymbolTableId) const;

        SymbolTableEntry& get_by_node_id(ID::ASTNodeId);
        const SymbolTableEntry& get_by_node_id(const ID::ASTNodeId) const;

        ID::SymbolTableId get_node_entry(ID::ASTNodeId) const;
};