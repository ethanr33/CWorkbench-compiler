
#include <iostream>

#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "codegen/AssemblyGenerator.h"

using std::cout;
using std::endl;
using std::string;

int main(int argc, char** argv) {
    if (argc != 2) {
        cout << "Usage: ./compiler <file name>" << endl;
        return 1;
    }

    Lexer lexer = Lexer();

    try {
        lexer.load_program(argv[1]);
        lexer.lex();
    } catch (const std::runtime_error& e) {
        std::cout << "Error while lexing: " << e.what() << std::endl;
        return 1;
    }

    Parser parser;

    try {
        parser.load_grammar("parser/grammar.txt");
        parser.generate_AST(lexer.token_stream);
    } catch (const std::runtime_error& e) {
        std::cout << "Error while parsing: " << e.what() << std::endl;
        return 1;
    }


    AssemblyGenerator generator;

    try {
        generator.convert_AST_to_assembly(parser.ast);
        generator.output_assembly_to_file("tmp/out.asm");
    } catch (const std::runtime_error& e) {
        std::cout << "Error during codegen: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}