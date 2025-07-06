
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <unordered_map>

#include "CFG.h"

using std::ifstream;
using std::istringstream;
using std::runtime_error;
using std::unordered_map;

// Function to match a token to a terminal symbol which generates the token
Symbol* CFG::token_to_symbol(Token& token) const {

    for (auto it = symbols.begin(); it != symbols.end(); it++) {
        Symbol* symbol = (*it).second;

        if (symbol->corresponding_token == TOKEN_TYPE::NON_TOKEN) {
            // This symbol is a non-terminal so we can't match it with a token
            continue;
        }

        if (symbol->corresponding_token == token.token_type) {
            return symbol;
        }
    }

    return nullptr;
}

void CFG::construct_FIRST_sets() {

    bool sets_changed;

    do {
        sets_changed = false;
        // For every symbol X (terminal or non-terminal)
        for (const auto& [symbol_string, symbol] : symbols) {
            int prev_size = symbol->FIRST.size();

            if (symbol->is_terminal) {
                // If X is a terminal, FIRST(X) = {X}
                symbol->FIRST.insert(symbol);
            } else {
                // The grammar for the language does not produce epsilon.
                // If this changes in the future, this algorithm will have to be updated.

                // Add case for if X produces epsilon here

                // If X -> Y1 Y2 .. Yk (which it will always be true if nothing produces epsilon)
                
                // Iterate through all productions of X

                for (const auto& rule : productions) {
                    if (rule.symbol->symbol == symbol->symbol) {
                        // If the production symbol for this rule matches,
                        // add Yi's FIRST set to the production's FIRST set
                        symbol->FIRST.insert(rule.production_rule.at(0)->FIRST.begin(), rule.production_rule.at(0)->FIRST.end());
                        
                    }
                }

                // If all Yi could derive epsilon, add epsilon to X's FIRST set
            }

            if (prev_size != symbol->FIRST.size()) {
                sets_changed = true;
            }
        }
    } while (sets_changed);

}

// Compute the FOLLOW sets for every nonterminal symbol in the grammar
// FOLLOW(X) is the set of all terminal characters which could immediately follow a production rule X
void CFG::construct_FOLLOW_sets() {
    bool sets_changed;

    // Create end of input symbol $, and add it to FOLLOw(S)
    // where S is the start production

    symbols.at("<root>")->FOLLOW.insert(symbols.at("$"));



    do {
        sets_changed = false;
        for (const auto& [symbol_string, symbol] : symbols) {
            int prev_size = symbol->FOLLOW.size();


            // if the symbol is terminal, it cannot have anything following it.
            // So its FOLLOw set is empty.
            // we can just skip over it
            if (symbol->is_terminal) {
                continue;
            }

            // for each production B -> alpha A beta:

            // Add FIRST(beta) to FOLLOW(A)

            for (const Rule& rule : productions) {
                Symbol* B = rule.symbol;
                

                bool has_nonterminal = false;

                for (const Symbol* rule_symbol : rule.production_rule) {
                    if (!rule_symbol->is_terminal) {
                        has_nonterminal = true;
                    }
                }   

                // If there is no nonterminal in this production, it is not in the form above, skip over this rule
                if (!has_nonterminal) {
                    continue;
                }

                // Find all instances in this rule which match the production above
                for (int i = 0; i < rule.production_rule.size(); i++) {
                    Symbol* A = rule.production_rule.at(i);

                    if (!A->is_terminal) {
                        Symbol* beta = nullptr;

                        if (i + 1 < rule.production_rule.size()) {
                            beta = rule.production_rule.at(i + 1);
                        }

                        // Add FIRST(beta) - {epsilon} to FOLLOW(A)
                        // Since we assume this grammar can't produce epsilon, the above expression simplifies to FIRST(beta)
                        // this grammar can't produce epsilon, so FIRST(beta) is the same as FIRST(b)
                        // where b is the nonterminal or terminal symbol that immediately follows after the first character of beta.
                        // If there is none, b is epsilon.

                        if (beta != nullptr) {
                            A->FOLLOW.insert(beta->FIRST.begin(), beta->FIRST.end());
                        }

                        // Next, if epsilon is in FIRST(beta) add FOLLOW(B) to FOLLOW(A)
                        // No production produces epsilon, so FIRST(beta) is only epsilon is beta is epsilon
                        // This is only the case if there is no production or terminal after A

                        if (beta == nullptr) {
                            A->FOLLOW.insert(B->FOLLOW.begin(), B->FOLLOW.end());
                        }


                    }
                }
            }


            if (prev_size != symbol->FOLLOW.size()) {
                sets_changed = true;
            }
        }
    } while (sets_changed);
}

void CFG::construct_parse_table() {
    // Iterate through every possible entry in the parse table T[A, a]
    // T[A, a] contains the index of A -> w iff a is in FIRST(w)

    for (int i = 0; i < productions.size(); i++) {
        Rule rule = productions.at(i);

        // Only nonterminal rules can be A
        if (!rule.is_terminal) {

            // Next, iterate through all terminal symbols
            for (const auto& [symbol_string, terminal_symbol]: symbols) {
                
                // Calculate what FIRST(w) is
                // We assume that the grammar does not produce epsilon, so FIRST(w) will be FIRST(x), where x is the first symbol in w

                unordered_set<Symbol*> FIRST_w = rule.production_rule.at(0)->FIRST;

                // Check if a is in FIRST(w)
                // If it is, then we can add the index of this rule into the parse table

                if (FIRST_w.find(terminal_symbol) != FIRST_w.end()) {
                    unordered_map<Symbol*, int> inner_map;

                    if (parse_table.find(rule.symbol) == parse_table.end()) {
                        inner_map = {{terminal_symbol, i}};
                        parse_table.insert({rule.symbol, inner_map});
                    } else {
                        parse_table.at(rule.symbol).at(terminal_symbol) = i;
                    }

                }

            }

            
        }
    }
}

bool is_nonterminal_symbol(const string& symbol) {
    return symbol.at(0) == '<' && symbol.at(symbol.size() - 1) == '>';
}

void CFG::generate_terminal_rules() {

    for (const auto& [symbol_string, symbol] : symbols) {
        if (symbol->is_terminal) {
            symbol->corresponding_token = (TOKEN_TYPE) TERMINAL_MAP.at(symbol->symbol);
            productions.push_back(TerminalRule(symbol, (TOKEN_TYPE) TERMINAL_MAP.at(symbol->symbol)));
        }
    }

}

void CFG::load_data(string file_path) {
    ifstream program_file(file_path);

    if (!program_file.is_open()) {
        throw runtime_error("Failed to open CFG file");
    }

    // Go line by line and add the rules for all non-terminal symbols

    string cur_line;
    while (getline(program_file, cur_line)) {
        istringstream ss = istringstream(cur_line);

        string production_nonterminal;
        ss >> production_nonterminal;

        if (symbols.find(production_nonterminal) == symbols.end()) {
            symbols.insert({production_nonterminal, new Symbol(production_nonterminal, false)});
        }

        string separator;
        ss >> separator;

        if (separator != SEPARATOR) {
            throw runtime_error("Found " + separator + " as separator, when it should be " + SEPARATOR);
        }

        vector<Symbol*> rule;

        string cur_token;
        while (ss >> cur_token) {
            Symbol* cur_symbol;

            if (!is_nonterminal_symbol(cur_token)) {
                cur_token = cur_token.substr(1, cur_token.length() - 2);
            }

            if (symbols.find(cur_token) != symbols.end()) {
                cur_symbol = symbols.at(cur_token);
            } else {
                cur_symbol = new Symbol(cur_token, !is_nonterminal_symbol(cur_token));
                symbols.insert({cur_token, cur_symbol});
            }

            rule.push_back(cur_symbol);

        }

        NonterminalRule new_rule(symbols.at(production_nonterminal), rule);

        productions.push_back(new_rule);
    }

    generate_terminal_rules();

    
    Symbol* end_of_input_symbol = new Symbol("$", true, TOKEN_TYPE::END_OF_INPUT);
    symbols.insert({end_of_input_symbol->symbol, end_of_input_symbol});
}   