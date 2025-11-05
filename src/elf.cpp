#include "elf.h"
#include <fstream>
#include <stdexcept>
#include <cstring>
#include <vector>

/**
 * @brief Writes the provided machine code into a minimal ELF64 object file.
 *
 * @param output_filename The name of the file to create (e.g., "output.o").
 * @param code A vector of bytes representing the machine code for the .text section.
 */
void write_elf_object_file(const std::string& output_filename, const std::vector<uint8_t>& code) {
    std::ofstream file(output_filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open output file for writing: " + output_filename);
    }

    // 1. Prepare the ELF Header
    Elf64_Ehdr ehdr = {0};
    
    // Set the ELF magic number and identification fields
    ehdr.e_ident[0] = 0x7f; ehdr.e_ident[1] = 'E'; ehdr.e_ident[2] = 'L'; ehdr.e_ident[3] = 'F';
    ehdr.e_ident[4] = 2;       // 2 = 64-bit format
    ehdr.e_ident[5] = 1;       // 1 = Little endian
    ehdr.e_ident[6] = 1;       // 1 = ELF version
    ehdr.e_ident[7] = 0;       // OS ABI (usually set to 0 for System V)
    
    ehdr.e_type = 1;           // 1 = ET_REL (Relocatable file, i.e., an object file)
    ehdr.e_machine = 62;       // 62 = EM_X86_64
    ehdr.e_version = 1;
    ehdr.e_entry = 0;          // No entry point in an object file
    ehdr.e_phoff = 0;          // No program headers
    ehdr.e_shoff = sizeof(Elf64_Ehdr); // Section headers start right after the ELF header
    ehdr.e_flags = 0;
    ehdr.e_ehsize = sizeof(Elf64_Ehdr);
    ehdr.e_phentsize = 0;
    ehdr.e_phnum = 0;
    ehdr.e_shentsize = sizeof(Elf64_Shdr);
    ehdr.e_shnum = 3;          // We will have 3 sections: NULL, .text, and .shstrtab
    ehdr.e_shstrndx = 2;       // The section header string table is at index 2

    // 2. Prepare the Section Header String Table data
    // Contains the names of our sections, null-terminated.
    // The table starts with a null byte.
    // \0.text\0.shstrtab\0
    const char shstrtab_data[] = "\0.text\0.shstrtab\0";
    
    // 3. Prepare the Section Headers
    Elf64_Shdr shdrs[3] = {0};

    // Section 0 is the NULL section, already zeroed out.

    // Section 1: .text section
    shdrs[1].sh_name = 1;      // Offset of ".text" in shstrtab_data
    shdrs[1].sh_type = 1;      // 1 = SHT_PROGBITS (Program data)
    shdrs[1].sh_flags = 6;     // 6 = SHF_ALLOC | SHF_EXECINSTR
    shdrs[1].sh_addr = 0;
    // The actual code will be written after the ELF header and all section headers
    shdrs[1].sh_offset = sizeof(Elf64_Ehdr) + (3 * sizeof(Elf64_Shdr));
    shdrs[1].sh_size = code.size();
    shdrs[1].sh_link = 0;
    shdrs[1].sh_info = 0;
    shdrs[1].sh_addralign = 16; // 16-byte alignment for code
    shdrs[1].sh_entsize = 0;

    // Section 2: .shstrtab section (the string table itself)
    shdrs[2].sh_name = 7;      // Offset of ".shstrtab" in shstrtab_data
    shdrs[2].sh_type = 3;      // 3 = SHT_STRTAB (String table)
    shdrs[2].sh_flags = 0;
    shdrs[2].sh_addr = 0;
    // The string table will be written right after the .text section's code
    shdrs[2].sh_offset = shdrs[1].sh_offset + code.size();
    shdrs[2].sh_size = sizeof(shstrtab_data);
    shdrs[2].sh_link = 0;
    shdrs[2].sh_info = 0;
    shdrs[2].sh_addralign = 1; // 1-byte alignment
    shdrs[2].sh_entsize = 0;

    // 4. Write everything to the file in the correct order
    // Write ELF Header
    file.write(reinterpret_cast<const char*>(&ehdr), sizeof(ehdr));
    // Write Section Headers
    file.write(reinterpret_cast<const char*>(shdrs), sizeof(shdrs));
    // Write .text section content (the actual machine code)
    file.write(reinterpret_cast<const char*>(code.data()), code.size());
    // Write .shstrtab section content
    file.write(shstrtab_data, sizeof(shstrtab_data));

    file.close();
}
