# KappaVM Examples

This directory contains example programs demonstrating KappaVM's features.

## Basic Examples

### `basic_arithmetic.kappa`
Demonstrates basic arithmetic operations with conditional jumps and labels.
```bash
./build/kappavm --assemble examples/basic_arithmetic.kappa basic_arithmetic.kbc
./build/kappavm basic_arithmetic.kbc
```

### `function_definition_only.kappa`
Shows how to define functions with FUNCTION/ENDFUNCTION syntax without calling them.
```bash
./build/kappavm --assemble examples/function_definition_only.kappa function_definition_only.kbc
./build/kappavm function_definition_only.kbc
```

## Function Call Examples

### `function_call_simple.kappa`
Simple function call example with correct calling convention:
- Function reference pushed first
- Arguments pushed after function
- CALL instruction with argument count

```bash
./build/kappavm --assemble examples/function_call_simple.kappa function_call_simple.kbc
./build/kappavm function_call_simple.kbc
# Expected output: 15
```

### `function_call_complex.kappa` 
More complex example with multiple function calls (note: uses incorrect calling convention for demonstration).

## Assembly Language Syntax

### Function Definitions
```kappa
FUNCTION function_name
  # Function body
  ADD
  RETURN
ENDFUNCTION
```

### Function Calls
```kappa
CONSTANT function_name  # Push function reference
CONSTANT arg1          # Push first argument  
CONSTANT arg2          # Push second argument
CALL 2                 # Call with 2 arguments
```

### Available Instructions
- `CONSTANT value` - Push constant onto stack
- `ADD` - Pop two values, push sum
- `CALL n` - Call function with n arguments
- `RETURN` - Return from function
- `JMP label` - Unconditional jump
- `JMP_IF_FALSE label` - Jump if top of stack is false/zero
- `HALT` - Stop execution

## Compilation and Execution

1. **Assemble**: Convert `.kappa` source to `.kbc` bytecode
   ```bash
   ./build/kappavm --assemble source.kappa output.kbc
   ```

2. **Execute**: Run bytecode file
   ```bash
   ./build/kappavm output.kbc
   ```

3. **Disassemble**: View bytecode contents
   ```bash
   ./build/kappavm --dis output.kbc
   ```