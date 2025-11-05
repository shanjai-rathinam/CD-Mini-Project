# Mini x86-64 Assembler

A simple two-pass assembler for a subset of the x86-64 assembly language, implemented in C++ as a compiler design mini-project.

## Features

*   **Two-Pass Design:** Correctly handles forward references for labels.
*   **Instruction Subset:** Assembles a core subset of x86-64 instructions (`MOV`, `ADD`, `SUB`, `JMP`, `SYSCALL`).
*   **ELF64 Object File Generation:** Produces a standard relocatable object file (`.o`) that can be linked with `ld`.
*   **Basic Syntax Support:** Understands labels, mnemonics, and operands, and supports `.text` and `.global` directives.
*   **Cross-Platform:** Written in standard C++17 with a `Makefile` for easy compilation on Linux and macOS.

## File Structure

The project is organized as follows:
mini-assembler/
├── Makefile              # Automates the build process
├── README.md             # This file
├── include/              # Header files
│   ├── assembler.h
│   └── elf.h
├── src/                  # Source code files
│   ├── main.cpp          # Main program driver
│   ├── pass1.cpp         # First pass: builds the symbol table
│   ├── pass2.cpp         # Second pass: generates machine code
│   └── elf.cpp           # ELF object file generation logic
└── examples/             # Example assembly files
    └── simple.asm

## Getting Started

Follow these instructions to build and run the assembler on your local machine.

### Prerequisites

You will need a C++17 compliant compiler and `make`. These are standard on most Linux distributions and can be installed on macOS with Xcode Command Line Tools.

*   `g++` (or another C++ compiler)
*   `make`
*   `ld` (the linker, for testing the output)

### Building

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/<your-username>/mini-assembler.git
    cd mini-assembler
    ```

2.  **Compile the project:**
    Use the provided `Makefile` to build the executable.
    ```bash
    make
    ```
    This will create an executable named `mini_assembler` in the root directory.

## Usage

1.  **Assemble an Assembly File:**
    Run the assembler on an example file. The `-o` flag specifies the output object file.
    ```bash
    ./mini_assembler examples/simple.asm -o simple.o
    ```

2.  **Link the Object File:**
    Use a standard linker like `ld` to create a final executable from the generated object file.
    ```bash
    ld simple.o -o simple_executable
    ```

3.  **Run the Executable:**
    ```bash
    ./simple_executable
    ```

4.  **Check the Exit Code:**
    The example program is designed to exit with code 42. You can verify this with:
    ```bash
    echo $?
    ```
    The output should be `42`.

## Assembler Design

This project implements a classic **two-pass assembler** to resolve the issue of forward references (using a label before it is defined).

*   **Pass 1:** The first pass scans the entire assembly source code file. Its only job is to build a **Symbol Table**. It calculates the memory address for every label it encounters and stores the `(label, address)` pair in the table. It uses a location counter to keep track of addresses as it moves through the instructions.

*   **Pass 2:** The second pass re-reads the source code from the beginning. This time, it translates each instruction mnemonic into its corresponding machine code opcode. When it encounters an operand that is a label, it looks up the label's address in the completed Symbol Table. It then assembles the opcodes and operands into binary machine code and writes it to an ELF object file.

## Example Assembly Code

The following is the content of `examples/simple.asm`, which the assembler can process.

```assembly
; This program sets an exit code and terminates.
section .text
global _start

_start:
    mov rax, 60     ; The syscall number for 'exit'
    mov rdi, 42     ; The exit code to return
    syscall         ; Call the kernel to exit
