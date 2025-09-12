
#include "Parser.h"

void Parser::generate_AST(const vector<Token>& token_stream) {
    ast.construct_parse_tree(token_stream);
    ast.construct_AST_from_parse_tree();
}