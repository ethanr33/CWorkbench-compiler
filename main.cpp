
#include <iostream>

#include "lexer/Lexer.h"
#include "codegen/SymbolTable.h"
#include "parser/Parser.h"

using std::cout;
using std::endl;
using std::string;

int main(int argc, char** argv) {
    if (argc != 2) {
        cout << "Usage: ./compiler <file name>" << endl;
        return 1;
    }

    SymbolTable symbol_table;

    Lexer lexer = Lexer();

    try {
        lexer.load_program(argv[1]);
        lexer.lex();
    } catch (const std::runtime_error& e) {
        std::cout << "Error while lexing: " << e.what() << std::endl;
        return 1;
    }

    CFG grammar;

    try {
        grammar.load_data("parser/grammar.txt");

        grammar.construct_FIRST_sets();
        grammar.construct_FOLLOW_sets();
        grammar.construct_parse_table();
    } catch (const std::runtime_error& e) {
        std::cout << "Error while constructing grammar: " << e.what() << std::endl;
        return 1;
    }

    Parser parser = Parser(symbol_table, grammar);

    try {
        parser.generate_AST(lexer.token_stream);
    } catch (const std::runtime_error& e) {
        std::cout << "Error while parsing: " << e.what() << std::endl;
        return 1;
    }

    parser.ast.print_AST();

    // AssemblyGenerator generator;

    // try {
    //     generator.convert_AST_to_assembly(parser.ast);
    //     generator.output_assembly_to_file("tmp/out.asm");
    // } catch (const std::runtime_error& e) {
    //     std::cout << "Error during codegen: " << e.what() << std::endl;
    //     return 1;
    // }

    return 0;
}