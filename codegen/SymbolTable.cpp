
#include "SymbolTable.h"

ID::SymbolTableId SymbolTable::add(ID::ASTNodeId node_id, const std::string& ident) {
    ID::SymbolTableId table_id = table_arena.add(SymbolTableEntry(ident, node_id));
    ident_to_entry.insert({ident, table_id});
    node_id_to_entry.insert({table_id, node_id})
}

SymbolTableEntry& SymbolTable::get(ID::SymbolTableId id) {
    return table_arena.get(id);
}

const SymbolTableEntry& SymbolTable::get(ID::SymbolTableId id) const {
    return table_arena.get(id);
}

SymbolTableEntry& SymbolTable::get(ID::ASTNodeId id) {
    return table_arena.get(get_node_entry(id));
}

const SymbolTableEntry& SymbolTable::get(ID::ASTNodeId id) const {
    return table_arena.get(get_node_entry(id));
}

ID::SymbolTableId SymbolTable::get_node_entry(ID::ASTNodeId node_id) const {
    if (node_id_to_entry.find(node_id) != node_id_to_entry.end()) {
        return node_id_to_entry.at(node_id);
    } else {
        return SymbolTable::invalid_entry;
    }
}