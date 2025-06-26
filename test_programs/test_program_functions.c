#include "assembler.h"
#include "vm.h"
#include <stdio.h>

int main() {
    const char *src =
        "FUNCTION add_numbers\n"
        "  ADD\n"
        "  RETURN\n"
        "ENDFUNCTION\n"
        "\n"
        "  CONSTANT 5\n"
        "  CONSTANT 10\n"
        "  CONSTANT add_numbers\n"
        "  CALL 2\n"
        "  HALT\n";

    printf("Assembling program with function definitions...\n");
    Program program = assemble_program_from_string(src);
    
    printf("Functions defined: %zu\n", program.function_count);
    if (program.function_count > 0) {
        printf("Function name: %s\n", program.functions[0].name);
        printf("Function code instructions: %zu\n", program.functions[0].chunk->code.count);
    }
    
    printf("Main chunk constants: %zu\n", program.main_chunk.constants.count);
    printf("Main chunk instructions: %zu\n", program.main_chunk.code.count);
    
    // Set up VM to run the main chunk
    VM vm;
    vm_init(&vm);
    
    CallFrame* frame = &vm.frames[vm.frame_count++];
    frame->chunk = &program.main_chunk;
    frame->ip = program.main_chunk.code.code;
    frame->slots = vm.stack;
    
    printf("Running program...\n");
    vm_run(&vm);
    
    printf("Result: %lld\n", vm.stack[0].as.number);
    
    vm_free(&vm);
    free_program(&program);
    
    return 0;
}