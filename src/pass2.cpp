#include "assembler.h"
#include "elf.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdint>
#include <algorithm>

void pass2(const std::string& input_filename, const std::string& output_filename, const SymbolTable& symtab) {
    std::ifstream input_file(input_filename);
    if (!input_file) {
        throw std::runtime_error("Cannot open input file: " + input_filename);
    }

    std::vector<uint8_t> machine_code;
    std::string line;

    while (std::getline(input_file, line)) {
        line.erase(0, line.find_first_not_of(" \t\n\r"));
        line.erase(line.find_last_not_of(" \t\n\r") + 1);

        if (line.empty() || line[0] == ';') continue;

        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;

        while (ss >> token) {
            token.erase(std::remove(token.begin(), token.end(), ','), token.end());
            tokens.push_back(token);
        }

        if (tokens.empty()) continue;

        if (tokens[0].back() == ':') {
            tokens.erase(tokens.begin());
        }

        if (tokens.empty()) continue;

        std::string mnemonic = tokens[0];
        std::vector<std::string> operands;
        if (tokens.size() > 1) {
            operands.assign(tokens.begin() + 1, tokens.end());
        }

        // --- Machine Code Generation ---
        if (mnemonic == "mov") {
            if (operands.size() == 2) {
                if (operands[0] == "rax") {
                    machine_code.push_back(0x48);
                    machine_code.push_back(0xb8);
                    uint64_t value = std::stoull(operands[1]);
                    for (int i = 0; i < 8; ++i) {
                        machine_code.push_back((value >> (i * 8)) & 0xff);
                    }
                } else if (operands[0] == "rdi") {
                    machine_code.push_back(0x48);
                    machine_code.push_back(0xbf);
                    uint64_t value = std::stoull(operands[1]);
                    for (int i = 0; i < 8; ++i) {
                        machine_code.push_back((value >> (i * 8)) & 0xff);
                    }
                }
            }
        } else if (mnemonic == "add") {
             // Simplified: add rax, imm8
            if (operands.size() == 2 && operands[0] == "rax") {
                machine_code.push_back(0x48);
                machine_code.push_back(0x05);
                uint8_t value = std::stoi(operands[1]);
                machine_code.push_back(value);
            }
        } else if (mnemonic == "sub") {
            // Simplified: sub rax, imm8
            if (operands.size() == 2 && operands[0] == "rax") {
                machine_code.push_back(0x48);
                machine_code.push_back(0x2d);
                uint8_t value = std::stoi(operands[1]);
                machine_code.push_back(value);
            }
        } else if (mnemonic == "jmp") {
            if (operands.size() == 1) {
                if (symtab.count(operands[0])) {
                    machine_code.push_back(0xeb); // Short jump
                    long target_addr = symtab.at(operands[0]);
                    long current_addr = machine_code.size() + 2; // after this instruction
                    int8_t offset = target_addr - current_addr;
                    machine_code.push_back(offset);
                } else {
                     throw std::runtime_error("Undefined label in jmp: " + operands[0]);
                }
            }
        } else if (mnemonic == "syscall") {
            machine_code.push_back(0x0f);
            machine_code.push_back(0x05);
        }
    }

    write_elf_object_file(output_filename, machine_code);
}
