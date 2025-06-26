# Test Programs

This directory contains C programs for testing KappaVM functionality.

## Programs

### `test_program_functions.c`
Demonstrates in-memory function creation and execution without serialization.
Tests the core VM functionality with functions.

```bash
gcc -o test_program_functions test_program_functions.c ../assembler.c ../chunk.c ../vm.c
./test_program_functions
```

### `test_manual_function.c` 
Demonstrates manual creation of a program with functions and proper serialization.
Tests the complete round-trip: create → save → load → execute.

```bash
gcc -o test_manual_function test_manual_function.c ../assembler.c ../chunk.c ../vm.c
./test_manual_function
```

Both programs should output `15` (5 + 10) demonstrating successful function calls.