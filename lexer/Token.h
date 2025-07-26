
#pragma once

#include <ostream>
#include <string>

using std::string;
using std::ostream;

/*

Glossary:

O - Open
C - Closed
KW - Keyword
CONST - Constant

*/

enum class TOKEN_TYPE {
    KW_RETURN,
    KW_INT,
    INT_CONST,
    IDENTIFIER,
    O_CURLY,
    C_CURLY,
    O_PAREN,
    C_PAREN,
    SEMICOLON,
    PLUS,
    EPSILON,
    END_OF_INPUT,
    NON_TOKEN
};

// Lets us print out the value for a TOKEN_TYPE
ostream& operator<<(ostream&, TOKEN_TYPE);

static const int NUM_TOKENS = 11;

struct Token {
    TOKEN_TYPE token_type;
    string value;

    Token(TOKEN_TYPE type, string value) : token_type(type), value(value) {}
    
    friend ostream& operator<<(ostream& out, const Token& token) {
        out << token.value;
        return out;
    }
};

// Given a token consisting of alphabet characters, determine what type of keyword or token it is
TOKEN_TYPE determine_token_type(const string&);
