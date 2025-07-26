
#pragma once

#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <memory>

#include "../lexer/Lexer.h"

using std::vector;
using std::string;
using std::unordered_set;
using std::unordered_map;

const string SEPARATOR = "::=";

// Specifies how tokens should be written in the grammar file
const std::unordered_map<std::string, TOKEN_TYPE> TERMINAL_MAP = {
    {"RETURN",    TOKEN_TYPE::KW_RETURN},
    {"INT",       TOKEN_TYPE::KW_INT},
    {"INT_CONST", TOKEN_TYPE::INT_CONST},
    {"IDENT",     TOKEN_TYPE::IDENTIFIER},
    {"{",         TOKEN_TYPE::O_CURLY},
    {"}",         TOKEN_TYPE::C_CURLY},
    {"(",         TOKEN_TYPE::O_PAREN},
    {")",         TOKEN_TYPE::C_PAREN},
    {";",         TOKEN_TYPE::SEMICOLON},
    {"$",         TOKEN_TYPE::END_OF_INPUT},
    {"ε",         TOKEN_TYPE::EPSILON},
    {"+",         TOKEN_TYPE::PLUS}
};

/**
 * @brief Represents a symbol, which can be a terminal or a nonterminal character
 */
struct Symbol {
    string symbol; // String representation of the symbol
    bool is_terminal; // True if the symbol is a terminal symbol

    unordered_set<Symbol*> FIRST; // The FIRST set of this symbol
    unordered_set<Symbol*> FOLLOW; // The FOLLOW set of this symbol

    TOKEN_TYPE corresponding_token = TOKEN_TYPE::NON_TOKEN; // The corresponding token for this symbol, NON_TOKEN if the symbol is nonterminal

    Symbol() : symbol(""), is_terminal(true) {}
    Symbol(string symbol, bool is_terminal) : symbol(symbol), is_terminal(is_terminal) {}
    Symbol(string symbol, bool is_terminal, TOKEN_TYPE t) : symbol(symbol), is_terminal(is_terminal), corresponding_token(t) {}

};

/**
 * @brief Represents a rule in the grammar.
 */
struct Rule {
    Symbol* symbol; // The nonterminal symbol which produces this rule
    vector<Symbol*> production_rule; // The symbols which make up this rule
    bool is_terminal; // True if this rule produces a single terminal symbol

    Rule() : symbol(nullptr) {}
    Rule(Symbol* symbol) : symbol(symbol) {}
    Rule(Symbol* symbol, vector<Symbol*> production_rule, bool is_terminal) : symbol(symbol), production_rule(production_rule), is_terminal(is_terminal) {}

};


/**
 * @brief Represents a rule which produces anything other than a single terminal symbol.
 */
struct NonterminalRule : Rule {

    NonterminalRule(Symbol* symbol, vector<Symbol*> rule) : Rule(symbol, rule, false) {
        is_terminal = false;
    }

};


/**
 * @brief Represents a rule which produces a single terminal symbol, which corresponds to a token.
 */
struct TerminalRule : Rule {
    TOKEN_TYPE terminal_symbol; // The token which is produces by this rule

    TerminalRule(Symbol* symbol, TOKEN_TYPE terminal_symbol) : Rule(symbol, {}, true), terminal_symbol(terminal_symbol) {}

};


/**
 * @brief Represents a context-free grammar.
 * 
 * Provides data members and methods for symbols, productions, and a generated parse table.
 */
struct CFG {

    unordered_map<string, Symbol*> symbols; // List of symbols for this grammar

    unordered_map<Symbol*, unordered_map<Symbol*, int>> parse_table; // The generated parse table
    
    vector<Rule> productions; // List of productions for this grammar

    string CFG_data; // Path to a file which contains the grammar's rules

    // Get CFG data represented in BNF form and store the raw text in CFG_data
    void load_data(const string&);
    void generate_terminal_rules();

    Symbol* token_to_symbol(Token&) const;
    
    void construct_FIRST_sets();
    void construct_FOLLOW_sets();
    void construct_parse_table();
};