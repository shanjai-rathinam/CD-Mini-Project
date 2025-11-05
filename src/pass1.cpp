#include "assembler.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

// A very simple function to estimate instruction size
// A real assembler would have a detailed opcode table.
int estimate_instruction_size(const std::string& mnemonic, const std::vector<std::string>& operands) {
    if (mnemonic == "mov") {
        // mov rax, <imm64> is 10 bytes
        if (operands.size() == 2 && operands[0] == "rax") return 10;
        // mov rdi, <imm64> is 10 bytes
        if (operands.size() == 2 && operands[0] == "rdi") return 10;
        // A simple default
        return 2;
    }
    if (mnemonic == "add" || mnemonic == "sub") return 3;
    if (mnemonic == "jmp") return 2; // Short jump
    if (mnemonic == "syscall") return 2;
    return 0; // For directives like .global or .text
}

SymbolTable pass1(const std::string& input_filename) {
    std::ifstream input_file(input_filename);
    if (!input_file) {
        throw std::runtime_error("Cannot open input file: " + input_filename);
    }

    SymbolTable symtab;
    long location_counter = 0;
    std::string line;

    while (std::getline(input_file, line)) {
        // Trim leading/trailing whitespace
        line.erase(0, line.find_first_not_of(" \t\n\r"));
        line.erase(line.find_last_not_of(" \t\n\r") + 1);

        if (line.empty() || line[0] == ';') continue; // Skip empty lines and comments

        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;

        while (ss >> token) {
            // Remove commas from operands
            token.erase(std::remove(token.begin(), token.end(), ','), token.end());
            tokens.push_back(token);
        }

        if (tokens.empty()) continue;

        // Check for a label
        if (tokens[0].back() == ':') {
            std::string label = tokens[0].substr(0, tokens[0].length() - 1);
            if (symtab.count(label)) {
                throw std::runtime_error("Duplicate label definition: " + label);
            }
            symtab[label] = location_counter;
            tokens.erase(tokens.begin()); // Remove the label from tokens
        }

        if (tokens.empty()) continue;

        // Process mnemonic and operands
        std::string mnemonic = tokens[0];
        std::vector<std::string> operands;
        if (tokens.size() > 1) {
            operands.assign(tokens.begin() + 1, tokens.end());
        }

        // For this mini-assembler, we'll only handle .text section
        if (mnemonic == "section" || mnemonic == ".global") {
             continue;
        }

        location_counter += estimate_instruction_size(mnemonic, operands);
    }

    return symtab;
}
