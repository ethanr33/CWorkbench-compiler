
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <unordered_map>

#include "CFG.h"

using std::ifstream;
using std::istringstream;
using std::runtime_error;
using std::unordered_map;

/**
 * @brief Given a token object, find its corresponding symbol in the language
 * 
 * @param token - The token object to find the symbol of
 * 
 * @return a pointer to the token's corresponding symbol
 * @return nullptr if the token does not have a corresponding symbol
 */
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

/**
 * @brief Construct the FIRST sets for every symbol
 * 
 * Each symbol object contains its own FIRST sets
 */
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

                // If X -> Y1 Y2 .. Yk 
                
                // Iterate through all productions of X

                bool produces_eps = false;

                for (const auto& rule : productions) {
                    if (rule.symbol->symbol == symbol->symbol) {
                        bool all_epsilon = true;

                        // Iterate through all Yi in X -> Y1 Y2 .. Yk
                        for (const auto& Y_i : rule.production_rule) {
                            // If Y_i produces epsilon, move on to Y_(i + 1)
                            // Otherwise we can add this symbol's FIRST set to X's FIRST set
                            if (Y_i->FIRST.find(symbols.at("ε")) == Y_i->FIRST.end()) {
                                all_epsilon = false;
                                symbol->FIRST.insert(Y_i->FIRST.begin(), Y_i->FIRST.end());
                                break;
                            }
                        }

                        // If all Y_i produces epsilon
                        if (all_epsilon) {
                            symbol->FIRST.insert(symbols.at("ε"));
                        }
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

/**
 * @brief Construct the FOLLOW sets for every symbol
 * 
 * FOLLOW sets are stored in each Symbol object
 */
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
                            unordered_set<Symbol*> beta_copy = beta->FIRST;
                            beta_copy.erase(symbols.at("ε"));
                            A->FOLLOW.insert(beta_copy.begin(), beta_copy.end());
                        }

                        // Next, if epsilon is in FIRST(beta) add FOLLOW(B) to FOLLOW(A)
                        // No production produces epsilon, so FIRST(beta) is only epsilon is beta is epsilon
                        // This is only the case if there is no production or terminal after A

                        if (beta == nullptr) {
                            A->FOLLOW.insert(B->FOLLOW.begin(), B->FOLLOW.end());
                        } else if (beta->FIRST.find(symbols.at("ε")) != beta->FIRST.end()) {
                            // epsilon is in FIRST(beta)
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

/**
 * @brief Construct the parse table using the symbols of the grammar.
 * 
 * Used for AST generation in the next stage of parsing.
 */
void CFG::construct_parse_table() {
    // Iterate through every possible entry in the parse table T[A, a]
    // T[A, a] contains the index of A -> w iff:
    // 1. a is in FIRST(w)
    // 2. epsilon is in FIRST(w) and A is in FOLLOW(A)

    // Iterate through all nonterminal symbols A
    for (const auto& [nonterminal_symbol_string, nonterminal_symbol] : symbols) {
        if (nonterminal_symbol->is_terminal) {
            continue;
        }

        // Next, iterate through all terminal symbols a
        for (const auto& [symbol_string, terminal_symbol]: symbols) {

            if (!terminal_symbol->is_terminal) {
                continue;
            }

            // Iterate through all A -> w
            for (int i = 0; i < productions.size(); i++) {
                Rule rule = productions.at(i);

                // Rule must be in the form A -> w for algorithm to work
                if (rule.symbol != nonterminal_symbol) {
                    continue;
                }

                // Only nonterminal rules can be A -> w
                if (!rule.is_terminal) {

                    // Calculate what FIRST(w) is by potentially iterating over each symbol

                    unordered_set<Symbol*> FIRST_w = rule.production_rule.at(0)->FIRST;
                    
                    // If the first symbol produces epsilon, then we must add the second symbol's FIRST set to FIRST(w)
                    // If the second symbol produces epsilon, then we add the third FIRST set, and so on
                    // until a symbol which doesn't produce epsilon is reached, or all symbols produce epsilon
                    // In that case, epsilon will be in FIRST(w)
                    if (FIRST_w.find(symbols.at("ε")) != FIRST_w.end()) {
                        for (int symbol_index = 1; symbol_index < rule.production_rule.size(); symbol_index++) {
                            Symbol* cur_symbol = rule.production_rule.at(symbol_index);
                            FIRST_w.insert(cur_symbol->FIRST.begin(), cur_symbol->FIRST.end());

                            if (cur_symbol->FIRST.find(symbols.at("ε")) == cur_symbol->FIRST.end()) {
                                FIRST_w.erase(FIRST_w.find(symbols.at("ε")));
                                break;
                            }
                        }
                    }


                    // Check if a is in FIRST(w)
                    // If it is, then we can add the index of this rule into the parse table

                    // Check if either of the conditions mentioned above are true
                    bool condition_1 = FIRST_w.find(terminal_symbol) != FIRST_w.end();
                    bool condition_2 = FIRST_w.find(symbols.at("ε")) != FIRST_w.end()
                                        && nonterminal_symbol->FOLLOW.find(terminal_symbol) != nonterminal_symbol->FOLLOW.end(); 

                    if (condition_1 || condition_2) {
                        unordered_map<Symbol*, int> inner_map;

                        if (parse_table.find(nonterminal_symbol) == parse_table.end()) {
                            inner_map = {{terminal_symbol, i}};
                            parse_table.insert({nonterminal_symbol, inner_map});
                        } else if (parse_table.at(nonterminal_symbol).find(terminal_symbol) == parse_table.at(nonterminal_symbol).end()) {
                            parse_table.at(nonterminal_symbol).insert({terminal_symbol, i});
                        } else {
                            throw runtime_error("Error when constructing parse table: an entry contains multiple rules");
                        }

                    }

                }
            }

        }
    } 
}

/**
 * @brief Determines if a symbol's string represents a nonterminal production
 * @param symbol - The symbol to check
 * @return true if the symbol is nonterminal, false otherwise
 */
bool is_nonterminal_symbol(const string& symbol) {
    return symbol.at(0) == '<' && symbol.at(symbol.size() - 1) == '>';
}

/**
 * @brief Generate rules for this grammar of the form R -> t, where t is a teminal symbol
 * Also generate a symbol for ε and the end of input symbol $ (which does not appear in the grammar)
 */
void CFG::generate_terminal_rules() {

    Symbol* end_of_input_symbol = new Symbol("$", true, TOKEN_TYPE::END_OF_INPUT);
    symbols.insert({end_of_input_symbol->symbol, end_of_input_symbol});

    Symbol* epsilon = new Symbol("ε", true, TOKEN_TYPE::EPSILON);
    symbols.insert({epsilon->symbol, epsilon});

    for (const auto& [symbol_string, symbol] : symbols) {
        if (symbol->is_terminal) {
            symbol->corresponding_token = TERMINAL_MAP.at(symbol->symbol);
            productions.push_back(TerminalRule(symbol, TERMINAL_MAP.at(symbol->symbol)));
        }
    }

}

/**
 * @brief Load in grammar data from a file path. In the process, generate the rules and symbols associated with the grammar.
 * The function loads in grammar data in BNF form. For each rule it finds, it generates a Symbol* for every symbol in the rule that doesn't exist.
 * Each rule should be in the form of: <production> ::= S1 S2 .. 
 * Each symbol in the production is a nonterminal. Some nonterminals will generate a single terminal symbol, which corresponds to a token. 
 * Each nonterminal which generates a single terminal symbol should be wrapped in " ". For example, ";". The symbol inside must correspond to a TOKEN_TYPE.
 * Each nonterminal which does not generate a single terminal symbol should be wrapped in < >. For example, <root>
 * 
 * The | character can be used to separate different rules. | cannot be used to denote an empty rule body.
 * The ε character can be used to denote the empty string.
 * 
 * @throws runtime error if the file cannot be opened
 * @throws runtime error if the parser encounters a rule which does not contain the correct separator
 * @throws runtime error if a terminal symbol does not correspond to a token
 * @throws runtime error if | is used to specify an empty rule
 * 
 * @param file_path - Path of the file containing the grammar
 */
void CFG::load_data(const string& file_path) {
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

            if (cur_token == "|") {
                if (rule.size() == 0) {
                    throw runtime_error("Error while parsing " + file_path + ": | cannot specify an empty rule");
                }

                NonterminalRule new_rule(symbols.at(production_nonterminal), rule);
                productions.push_back(new_rule);
                rule.clear();
                continue;
            }

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
}   