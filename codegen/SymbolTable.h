
#pragma once

#include <string>
#include <unordered_map>

#include "../tools/memory.h"

enum class ENTRY_TYPE {
    FUNCTION,
    VARIABLE
};

struct SymbolTableEntry {
    ENTRY_TYPE entry_type;
    std::string identifier;
    ID::ASTNodeId node_id;

    uint32_t offset;

    SymbolTableEntry(const std::string& identifier, ID::ASTNodeId node_id, ENTRY_TYPE type) : identifier(identifier), node_id(node_id), entry_type(type) {}
};

class SymbolTable {
    private:
        Arena<SymbolTableEntry> table_arena;
        std::unordered_map<std::string, ID::SymbolTableId> ident_to_entry;
        std::unordered_map<ID::ASTNodeId, ID::SymbolTableId> node_id_to_entry;
    public:

        static constexpr ID::SymbolTableId invalid_entry = -1;

        ID::SymbolTableId add_function(ID::ASTNodeId, const std::string& string);
        ID::SymbolTableId add_variable(ID::ASTNodeId, const std::string& string);

        bool has_identifier(const std::string&) const;

        SymbolTableEntry& get_by_identifier(const std::string&);
        const SymbolTableEntry& get_by_identifier(const std::string&) const;

        SymbolTableEntry& get_by_node_id(ID::ASTNodeId);
        const SymbolTableEntry& get_by_node_id(const ID::ASTNodeId) const;
};