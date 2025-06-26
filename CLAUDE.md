# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

KappaVM is a stack-based, high-performance virtual machine written in C11. It executes custom bytecode and includes a complete toolchain with assembler, disassembler, and VM execution engine.

## Build System

**Primary Build Tool**: CMake (minimum version 3.30)

### Common Commands

```bash
# Initial setup
mkdir build && cd build
cmake ..

# Build all targets
make

# Run all tests
make test

# Build and run specific executable
make kappavm
./kappavm [options] [file]
```

### Available Executables

- `kappavm` - Main VM executable with CLI interface
- `vm_tests`, `opcode_tests`, `execution_tests`, `chunk_tests`, `cli_test`, `assembler_tests` - Test executables

## Usage Patterns

```bash
# Execute bytecode file
./kappavm file.kbc

# Assemble source to bytecode
./kappavm --assemble source.kappa output.kbc

# Disassemble bytecode to assembly
./kappavm --dis bytecode.kbc > disassembled.kappa
```

## Architecture Overview

### Core Components

1. **VM Core** (`vm.h`, `vm.c`): Stack-based execution engine
   - 256-value stack (VM_INIT_STACK_SIZE)
   - 64 max concurrent call frames (MAX_FRAMES)
   - Call frame-based function calls

2. **Instruction Set** (`opcode.h`): 64-bit instructions
   - 8-bit opcode (upper byte)
   - 56-bit operand data (lower 7 bytes)
   - Available ops: CONSTANT, ADD, HALT, JMP_IF_FALSE, JMP, CALL, RETURN

3. **Value System** (`value.h`): Tagged union type system
   - Types: VAL_NULL, VAL_NUMBER, VAL_STRING, VAL_LIST, VAL_OBJECT, VAL_FUNCTION
   - First-class functions and prototype-based objects

4. **Bytecode Management** (`chunk.h`, `chunk.c`): Code and constant storage
   - Binary serialization/deserialization
   - Dynamic growth for code and constants
   - Built-in disassembly support

5. **Assembler** (`assembler.h`, `assembler.c`): Assembly compilation
   - Input: `.kappa` files (human-readable assembly)
   - Output: `.kbc` files (binary bytecode)
   - Label resolution for jumps

### Assembly Language Syntax

Kappa assembly uses simple instruction format:
```
CONSTANT 1
CONSTANT 1
JMP_IF_FALSE end
CONSTANT 10
JMP finish
end:
CONSTANT 20
finish:
ADD
HALT
```

## Testing Framework

**Framework**: Custom lightweight testing framework using `tests/test_macros.h`

**Test Coverage**: 6 test suites covering all major components
- VM execution, opcodes, chunks, CLI, assembler functionality

**Running Tests**: Use `make test` - all tests must pass before committing changes.

## File Organization

### Core Implementation
- `main.c` - CLI entry point and argument parsing
- `vm.c` - Virtual machine execution engine
- `chunk.c` - Bytecode chunk management and serialization
- `assembler.c` - Assembly language compilation
- `common.h` - Shared types and includes

### Key Headers
- `vm.h` - VM interface and CallFrame structure
- `value.h` - Type system definitions and value operations
- `opcode.h` - Instruction set definitions
- `chunk.h` - Bytecode structure definitions
- `assembler.h` - Assembler interface

## Examples and Testing

### Examples Directory (`examples/`)
- `basic_arithmetic.kappa` - Basic operations with jumps and labels
- `function_definition_only.kappa` - Function definition syntax
- `function_call_simple.kappa` - Working function call with correct calling convention
- `function_call_complex.kappa` - Multiple function calls (demonstration)
- `README.md` - Complete usage documentation

### Test Programs (`test_programs/`)
- `test_program_functions.c` - In-memory function testing
- `test_manual_function.c` - Function serialization testing

## Development Notes

- **C Standard**: C11 required
- **Compiler**: GCC or Clang
- **Memory Management**: Manual memory management with proper cleanup
- **Error Handling**: Return-code based error handling throughout
- **Code Style**: Consistent C style with clear separation of concerns
- **Performance**: Optimized for execution speed with minimal runtime overhead
- **File Extensions**: `.kappa` for source, `.kbc` for bytecode (ignored by git)