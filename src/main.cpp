#include "assembler.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 4 || std::string(argv[2]) != "-o") {
        std::cerr << "Usage: " << argv[0] << " <input.asm> -o <output.o>" << std::endl;
        return 1;
    }

    std::string input_filename = argv[1];
    std::string output_filename = argv[3];

    try {
        // First Pass: Build the symbol table
        SymbolTable symtab = pass1(input_filename);

        // Second Pass: Generate machine code and write the object file
        pass2(input_filename, output_filename, symtab);

        std::cout << "Assembled " << input_filename << " to " << output_filename << " successfully." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
