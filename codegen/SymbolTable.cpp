
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

SymbolTableEntry& SymbolTable::get_by_identifier(const std::string& ident) {
    return table_arena.get(ident_to_entry.at(ident));
}

const SymbolTableEntry& SymbolTable::get_by_identifier(const std::string& ident) const {
    return table_arena.get(ident_to_entry.at(ident));
}


SymbolTableEntry& SymbolTable::get_by_node_id(ID::ASTNodeId id) {
    return table_arena.get(node_id_to_entry.at(id));
}

const SymbolTableEntry& SymbolTable::get_by_node_id(const ID::ASTNodeId id) const {
    return table_arena.get(node_id_to_entry.at(id));
}

bool SymbolTable::has_identifier(const std::string& ident) const {
    return ident_to_entry.find(ident) != ident_to_entry.end();
}