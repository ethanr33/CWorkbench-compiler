
#include <gtest/gtest.h>
#include <utility>

#include "../parser/CFG.h"

class CFGTest : public ::testing::Test {

    protected:
        CFG cfg;

        CFGTest() {
            cfg.load_data("../tests/simplegrammar.txt");
            cfg.generate_terminal_rules();
        }
};

TEST_F(CFGTest, ParseTableGenerationWorks) {
    cfg.construct_FIRST_sets();
    cfg.construct_FOLLOW_sets();
    cfg.construct_parse_table();

    SymbolId root_nonterminal = cfg.symbols.at("<root>");
    SymbolId func_nonterminal = cfg.symbols.at("<function>");
    SymbolId type_nonterminal = cfg.symbols.at("<type>");
    SymbolId return_nonterminal = cfg.symbols.at("<return>");

    SymbolId int_kw = cfg.symbols.at("INT");
    SymbolId return_kw = cfg.symbols.at("RETURN");

    if (cfg.parse_table.find(root_nonterminal) == cfg.parse_table.end()) {
        FAIL() << "Could not find any <root> entry in parse table";
    }

    if (cfg.parse_table.find(func_nonterminal) == cfg.parse_table.end()) {
        FAIL() << "Could not find any <function> entry in parse table";
    }

    if (cfg.parse_table.find(type_nonterminal) == cfg.parse_table.end()) {
        FAIL() << "Could not find any <type> entry in parse table";
    }

    if (cfg.parse_table.find(return_nonterminal) == cfg.parse_table.end()) {
        FAIL() << "Could not find any <return> entry in parse table";
    }



    if (cfg.parse_table.at(root_nonterminal).find(int_kw) == cfg.parse_table.at(root_nonterminal).end()) {
        FAIL() << "Could not find any <root> and INT entry in parse table";
    }

    if (cfg.parse_table.at(func_nonterminal).find(int_kw) == cfg.parse_table.at(func_nonterminal).end()) {
        FAIL() << "Could not find any <function> and INT entry in parse table";
    }

    if (cfg.parse_table.at(type_nonterminal).find(int_kw) == cfg.parse_table.at(type_nonterminal).end()) {
        FAIL() << "Could not find any <type> and INT entry in parse table";
    }

    if (cfg.parse_table.at(return_nonterminal).find(return_kw) == cfg.parse_table.at(return_nonterminal).end()) {
        FAIL() << "Could not find any <return> and RETURN entry in parse table";
    }

    EXPECT_EQ(cfg.parse_table.at(root_nonterminal).at(int_kw), 0);
    EXPECT_EQ(cfg.parse_table.at(func_nonterminal).at(int_kw), 1);
    EXPECT_EQ(cfg.parse_table.at(type_nonterminal).at(int_kw), 2);
    EXPECT_EQ(cfg.parse_table.at(return_nonterminal).at(return_kw), 3);


}

TEST_F(CFGTest, TokenToSymbolWorks) {

    // The last token is always a NON_TOKEN, which is not a real token so we don't test for it
    for (int i = 0; i < NUM_TOKENS - 1; i++) {
        Token token((TOKEN_TYPE) i, "");
        SymbolId symbol = cfg.token_to_symbol(token);

        bool symbol_exists = false;

        for (const auto& [symbol_string, symbol] : cfg.symbols) {
            if (cfg.symbol_arena.get(symbol).corresponding_token == (TOKEN_TYPE) i) {
                symbol_exists = true;
            }   
        }

        if (!symbol_exists) {
            continue;
        }

        if (symbol == Arena<Symbol>::invalid_id) {
            FAIL() << "token_to_symbol did not find anything for token " << i;
        }

        EXPECT_EQ(token.token_type, cfg.symbol_arena.get(symbol).corresponding_token) << "Failed to find matching symbol for token " << i;
    }

}

TEST_F(CFGTest, TokenToSymbolReturnsNull) {

    Token token(TOKEN_TYPE::NON_TOKEN, "invalid");
    SymbolId symbol = cfg.token_to_symbol(token);

    EXPECT_EQ(symbol, Arena<Symbol>::invalid_id) << "Expected token_to_symbol to not find NON_TOKEN, found " << cfg.symbol_arena.get(symbol).corresponding_token;

}

TEST_F(CFGTest, TokenToSymbolIgnoresTokenString) {

    Token token1(TOKEN_TYPE::IDENTIFIER, "a");
    SymbolId symbol1 = cfg.token_to_symbol(token1);

    Token token2(TOKEN_TYPE::IDENTIFIER, "otherident");
    SymbolId symbol2 = cfg.token_to_symbol(token2);

    EXPECT_EQ(token1.token_type, cfg.symbol_arena.get(symbol1).corresponding_token) << "Expected token_to_symbol to find identifier with string value a, found " << cfg.symbol_arena.get(symbol1).corresponding_token;
    EXPECT_EQ(token2.token_type, cfg.symbol_arena.get(symbol2).corresponding_token) << "Expected token_to_symbol to find identifier with string value a, found " << cfg.symbol_arena.get(symbol2).corresponding_token;

}