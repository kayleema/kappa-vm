#include "assembler.h"
#include "vm.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    // Create a program manually with correct function call
    Program program;
    init_chunk(&program.main_chunk);
    program.functions = NULL;
    program.function_count = 0;
    program.function_capacity = 0;

    // Create a simple add function
    Chunk* func_chunk = malloc(sizeof(Chunk));
    init_chunk(func_chunk);
    write_instruction(func_chunk, make_instruction(OP_ADD, 0));
    write_instruction(func_chunk, make_instruction(OP_RETURN, 0));
    
    Function* func = malloc(sizeof(Function));
    func->chunk = func_chunk;
    
    // Add constants to main chunk in correct order
    size_t func_idx = add_constant(&program.main_chunk, (Value){.type = VAL_FUNCTION, .as.function = func});
    size_t arg1_idx = add_constant(&program.main_chunk, (Value){.type = VAL_NUMBER, .as.number = 5});
    size_t arg2_idx = add_constant(&program.main_chunk, (Value){.type = VAL_NUMBER, .as.number = 10});
    
    // Create instructions with correct calling convention
    write_instruction(&program.main_chunk, make_instruction(OP_CONSTANT, func_idx));  // Push function
    write_instruction(&program.main_chunk, make_instruction(OP_CONSTANT, arg1_idx)); // Push arg1
    write_instruction(&program.main_chunk, make_instruction(OP_CONSTANT, arg2_idx)); // Push arg2
    write_instruction(&program.main_chunk, make_instruction(OP_CALL, 2));            // Call with 2 args
    write_instruction(&program.main_chunk, make_instruction(OP_HALT, 0));            // Halt
    
    printf("Saving manually created function program...\n");
    save_chunk(&program.main_chunk, "test_manual_function.kbc");
    
    printf("Loading and running saved program...\n");
    
    // Load and run the saved program
    Chunk loaded_chunk;
    init_chunk(&loaded_chunk);
    if (load_chunk(&loaded_chunk, "test_manual_function.kbc") != 0) {
        printf("Failed to load!\n");
        return 1;
    }
    
    VM vm;
    vm_init(&vm);
    
    CallFrame* frame = &vm.frames[vm.frame_count++];
    frame->chunk = &loaded_chunk;
    frame->ip = loaded_chunk.code.code;
    frame->slots = vm.stack;
    
    vm_run(&vm);
    
    printf("Result: %lld\n", vm.stack[0].as.number);
    
    vm_free(&vm);
    free_chunk(&loaded_chunk);
    free_chunk(func_chunk);
    free(func_chunk);
    free(func);
    free_chunk(&program.main_chunk);
    
    return 0;
}