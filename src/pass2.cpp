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
```---

#### `src/elf.cpp`

A simplified ELF file generator.

```cpp
#include "elf.h"
#include <fstream>
#include <cstring>

void write_elf_object_file(const std::string& output_filename, const std::vector<uint8_t>& code) {
    std::ofstream file(output_filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open output file for writing: " + output_filename);
    }

    Elf64_Ehdr ehdr = {0};
    // ELF magic number
    ehdr.e_ident[0] = 0x7f; ehdr.e_ident[1] = 'E'; ehdr.e_ident[2] = 'L'; ehdr.e_ident[3] = 'F';
    ehdr.e_ident[4] = 2; // 64-bit
    ehdr.e_ident[5] = 1; // Little endian
    ehdr.e_ident[6] = 1; // ELF version
    ehdr.e_type = 1;     // Relocatable file (object file)
    ehdr.e_machine = 62; // x86-64
    ehdr.e_version = 1;
    ehdr.e_entry = 0;
    ehdr.e_phoff = 0;
    ehdr.e_shoff = sizeof(Elf64_Ehdr); // Section headers start right after ELF header
    ehdr.e_flags = 0;
    ehdr.e_ehsize = sizeof(Elf64_Ehdr);
    ehdr.e_phentsize = 0;
    ehdr.e_phnum = 0;
    ehdr.e_shentsize = sizeof(Elf64_Shdr);
    ehdr.e_shnum = 3; // We will have 3 sections: NULL, .text, .shstrtab
    ehdr.e_shstrndx = 2; // Section header string table is at index 2

    // --- Section Headers ---
    Elf64_Shdr shdrs[3] = {0};

    // 1. .text section
    shdrs[1].sh_name = 1; // Offset into string table for ".text"
    shdrs[1].sh_type = 1; // SHT_PROGBITS
    shdrs[1].sh_flags = 6; // SHF_ALLOC | SHF_EXECINSTR
    shdrs[1].sh_addr = 0;
    shdrs[1].sh_offset = sizeof(Elf64_Ehdr) + 3 * sizeof(Elf64_Shdr);
    shdrs[1].sh_size = code.size();
    shdrs[1].sh_addralign = 16;

    // Section header string table
    const char shstrtab_data[] = "\0.text\0.shstrtab\0";

    // 2. .shstrtab section
    shdrs[2].sh_name = 7; // Offset for ".shstrtab"
    shdrs[2].sh_type = 3; // SHT_STRTAB
    shdrs[2].sh_flags = 0;
    shdrs[2].sh_addr = 0;
    shdrs[2].sh_offset = shdrs[1].sh_offset + code.size();
    shdrs[2].sh_size = sizeof(shstrtab_data);
    shdrs[2].sh_addralign = 1;

    // --- Writing to file ---
    // ELF Header
    file.write(reinterpret_cast<const char*>(&ehdr), sizeof(ehdr));
    // Section Headers
    file.write(reinterpret_cast<const char*>(shdrs), sizeof(shdrs));
    // .text section content
    file.write(reinterpret_cast<const char*>(code.data()), code.size());
    // .shstrtab content
    file.write(shstrtab_data, sizeof(shstrtab_data));

    file.close();
}
