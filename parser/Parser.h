
#include "CFG.h"
#include "AST.h"
#include "../codegen/SymbolTable.h"

struct Parser {
    CFG grammar;
    AST ast;
    SymbolTable& symbol_table;

    Parser(SymbolTable& table, CFG& grammar) : grammar(grammar), symbol_table(table), ast(table, grammar) {}
    
    // Given a stream of tokens, generate an AST which represents it
    // Store it within the ast variable
    void generate_AST(const vector<Token>&);

};