
#include "SymbolTable.h"

ID::SymbolTableId SymbolTable::add(ID::ASTNodeId id, const std::string& ident) {
    return table_arena.add(SymbolTableEntry(ident, id));
}

SymbolTableEntry& SymbolTable::get(ID::SymbolTableId id) {
    return table_arena.get(id);
}

const SymbolTableEntry& SymbolTable::get(ID::SymbolTableId id) const {
    return table_arena.get(id);
}