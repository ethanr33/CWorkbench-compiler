
#include "SymbolTable.h"

ID::SymbolTableId SymbolTable::add_function(ID::ASTNodeId node_id, const std::string& ident) {
    ID::SymbolTableId table_id = table_arena.add(SymbolTableEntry(ident, node_id, ENTRY_TYPE::FUNCTION));
    ident_to_entry.insert({ident, table_id});
    node_id_to_entry.insert({node_id, table_id});
    return table_id;
}

ID::SymbolTableId SymbolTable::add_variable(ID::ASTNodeId node_id, const std::string& ident) {
    ID::SymbolTableId table_id = table_arena.add(SymbolTableEntry(ident, node_id, ENTRY_TYPE::VARIABLE));
    ident_to_entry.insert({ident, table_id});
    node_id_to_entry.insert({node_id, table_id});
    return table_id;
}


SymbolTableEntry& SymbolTable::get(ID::SymbolTableId id) {
    return table_arena.get(id);
}

const SymbolTableEntry& SymbolTable::get(const ID::SymbolTableId id) const {
    return table_arena.get(id);
}

SymbolTableEntry& SymbolTable::get_by_node_id(ID::ASTNodeId id) {
    ID::SymbolTableId entry = get_node_entry(id);

    return table_arena.get(get_node_entry(id));
}

const SymbolTableEntry& SymbolTable::get_by_node_id(const ID::ASTNodeId id) const {
    return table_arena.get(get_node_entry(id));
}

ID::SymbolTableId SymbolTable::get_node_entry(ID::ASTNodeId node_id) const {
    if (node_id_to_entry.find(node_id) != node_id_to_entry.end()) {
        return node_id_to_entry.at(node_id);
    } else {
        return SymbolTable::invalid_entry;
    }
}