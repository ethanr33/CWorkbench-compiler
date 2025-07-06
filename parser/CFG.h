
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
    {"$",         TOKEN_TYPE::END_OF_INPUT}
};

struct Symbol {
    string symbol;
    bool is_terminal;

    unordered_set<Symbol*> FIRST;
    unordered_set<Symbol*> FOLLOW;

    TOKEN_TYPE corresponding_token = TOKEN_TYPE::NON_TOKEN;

    Symbol() : symbol(""), is_terminal(true) {}
    Symbol(string symbol, bool is_terminal) : symbol(symbol), is_terminal(is_terminal) {}
    Symbol(string symbol, bool is_terminal, TOKEN_TYPE t) : symbol(symbol), is_terminal(is_terminal), corresponding_token(t) {}

};

struct Rule {
    Symbol* symbol;
    vector<Symbol*> production_rule;
    bool is_terminal;

    Rule() : symbol(nullptr) {}
    Rule(Symbol* symbol) : symbol(symbol) {}
    Rule(Symbol* symbol, vector<Symbol*> production_rule, bool is_terminal) : symbol(symbol), production_rule(production_rule), is_terminal(is_terminal) {}

};

struct NonterminalRule : Rule {

    NonterminalRule(Symbol* symbol, vector<Symbol*> rule) : Rule(symbol, rule, false) {
        is_terminal = false;
    }

};


struct TerminalRule : Rule {
    TOKEN_TYPE terminal_symbol;

    TerminalRule(Symbol* symbol, TOKEN_TYPE terminal_symbol) : Rule(symbol, {}, true), terminal_symbol(terminal_symbol) {}

};


struct CFG {

    unordered_map<string, Symbol*> symbols;

    unordered_map<Symbol*, unordered_map<Symbol*, int>> parse_table;
    
    vector<Rule> productions;

    string CFG_data;

    // Get CFG data represented in BNF form and store the raw text in CFG_data
    void load_data(string);
    void generate_terminal_rules();

    Symbol* token_to_symbol(Token&) const;
    
    void construct_FIRST_sets();
    void construct_FOLLOW_sets();
    void construct_parse_table();
};