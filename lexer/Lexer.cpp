
#include <fstream>
#include <stdexcept>

#include "Lexer.h"

using std::ifstream;
using std::runtime_error;

void Lexer::load_program(const string& source) {
    ifstream program_file(source);

    if (!program_file.is_open()) {
        throw runtime_error("Failed to open input file");
    }

    string cur_line;
    while (getline(program_file, cur_line)) {
        // Manually adding a new line between lines makes it clear
        // to the lexer that identifiers cannot span multiple olines
        program_data += cur_line + '\n';
    }
}

// Assume that the program data is already in program_data
void Lexer::lex() {
    for (int i = 0; i < program_data.length(); i++) {

        if (program_data.at(i) == ';') {
            token_stream.push_back(Token(TOKEN_TYPE::SEMICOLON, ";"));
        } else if (program_data.at(i) == '(') {
            token_stream.push_back(Token(TOKEN_TYPE::O_PAREN, "("));
        } else if (program_data.at(i) == ')') {
            token_stream.push_back(Token(TOKEN_TYPE::C_PAREN, ")"));
        } else if (program_data.at(i) == '{') {
            token_stream.push_back(Token(TOKEN_TYPE::O_CURLY, "{"));
        } else if (program_data.at(i) == '}') {
            token_stream.push_back(Token(TOKEN_TYPE::C_CURLY, "}"));
        } else if (program_data.at(i) == '+') {
            token_stream.push_back(Token(TOKEN_TYPE::PLUS, "+"));
        } else if (program_data.at(i) == '*') {
            token_stream.push_back(Token(TOKEN_TYPE::ASTERISK, "*"));
        } else if (program_data.at(i) == '=') {
            token_stream.push_back(Token(TOKEN_TYPE::EQUALS, "="));
        } else if (program_data.at(i) == '-') {
            token_stream.push_back(Token(TOKEN_TYPE::HYPHEN, "-"));
        } else if (isdigit(program_data.at(i))) {
            // If we found a number, continue checking the string
            // until a non-numerical character is found

            string num_token;

            int j = i;
            while (j < program_data.length() && isdigit(program_data.at(j))) {
                num_token += program_data.at(j);
                j++;
            }

            token_stream.push_back(Token(TOKEN_TYPE::INT_CONST, num_token));

            // After the loop, j points to the char after the const number
            // Decrement by one to avoid skipping a character when the for loop increments i
            i = --j;
        } else if (isalpha(program_data.at(i)) || program_data.at(i) == '_') {
            string alpha_token;

            int j = i;
            while (j < program_data.length() && (isalnum(program_data.at(j)) || program_data.at(j) == '_')) {
                alpha_token += program_data.at(j);
                j++;
            }

            token_stream.push_back(Token(determine_token_type(alpha_token), alpha_token));

            // After the loop, j points to the char after the const number
            // Decrement by one to avoid skipping a character when the for loop increments i
            i = --j;
        }
    }

    token_stream.push_back(Token(TOKEN_TYPE::END_OF_INPUT, "$"));

}