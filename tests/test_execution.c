#include "../vm.h"
#include "../chunk.h"
#include "../opcode.h"
#include "test_macros.h"
#include <stdlib.h>

TEST(test_simple_addition) {
    VM vm;
    vm_init(&vm);

    Chunk chunk;
    init_chunk(&chunk);

    size_t const1 = add_constant(&chunk, (Value){.type = VAL_NUMBER, .as.number = 5});
    size_t const2 = add_constant(&chunk, (Value){.type = VAL_NUMBER, .as.number = 10});

    write_instruction(&chunk, make_instruction(OP_CONSTANT, const1));
    write_instruction(&chunk, make_instruction(OP_CONSTANT, const2));
    write_instruction(&chunk, make_instruction(OP_ADD, 0));
    write_instruction(&chunk, make_instruction(OP_HALT, 0));

    CallFrame* frame = &vm.frames[vm.frame_count++];
    frame->chunk = &chunk;
    frame->ip = chunk.code.code;
    frame->slots = vm.stack;
    vm_run(&vm);

    Value result = vm.stack[0];
    ASSERT_EQ(result.type, VAL_NUMBER, "%d");
    ASSERT_EQ(result.as.number, (int64_t)15, "%lld");

    vm_free(&vm);
    free_chunk(&chunk);
}

TEST(test_jmp_if_false) {
    VM vm;
    vm_init(&vm);
    Chunk chunk;
    init_chunk(&chunk);
    // Program: push 0, jmp_if_false to add 20, add 10
    add_constant(&chunk, (Value){.type = VAL_NUMBER, .as.number = 0});
    add_constant(&chunk, (Value){.type = VAL_NUMBER, .as.number = 10});
    add_constant(&chunk, (Value){.type = VAL_NUMBER, .as.number = 20});
    write_instruction(&chunk, make_instruction(OP_CONSTANT, 0)); // push 0 (false)
    write_instruction(&chunk, make_instruction(OP_JMP_IF_FALSE, 1)); // jump to inst 3
    write_instruction(&chunk, make_instruction(OP_CONSTANT, 1)); // push 10 (skipped)
    write_instruction(&chunk, make_instruction(OP_CONSTANT, 2)); // push 20
    write_instruction(&chunk, make_instruction(OP_HALT, 0));
    CallFrame* frame = &vm.frames[vm.frame_count++];
    frame->chunk = &chunk;
    frame->ip = chunk.code.code;
    frame->slots = vm.stack;
    vm_run(&vm);
    ASSERT_EQ(vm.stack[0].as.number, (int64_t)20, "%lld");
    ASSERT_EQ(vm.stack_top - vm.stack, (size_t)1, "%zu");
    free_chunk(&chunk);
    vm_free(&vm);
}

TEST(test_unconditional_jump) {
    VM vm;
    vm_init(&vm);
    Chunk chunk;
    init_chunk(&chunk);
    // Program: push 10, jump over pushing 20, halt
    add_constant(&chunk, (Value){.type = VAL_NUMBER, .as.number = 10});
    add_constant(&chunk, (Value){.type = VAL_NUMBER, .as.number = 20});
    write_instruction(&chunk, make_instruction(OP_CONSTANT, 0));    // 0: push 10
    write_instruction(&chunk, make_instruction(OP_JMP, 1));          // 1: jump to instruction 3
    write_instruction(&chunk, make_instruction(OP_CONSTANT, 1));    // 2: push 20 (should be skipped)
    write_instruction(&chunk, make_instruction(OP_HALT, 0));         // 3: halt
    CallFrame* frame = &vm.frames[vm.frame_count++];
    frame->chunk = &chunk;
    frame->ip = chunk.code.code;
    frame->slots = vm.stack;
    vm_run(&vm);

    ASSERT_EQ(vm.stack_top - vm.stack, (size_t)1, "Stack should have one value, but has %zu");
    ASSERT_EQ(vm.stack[0].as.number, (int64_t)10, "The value on stack should be 10, but was %lld");

    free_chunk(&chunk);
    vm_free(&vm);
}

TEST(test_function_call) {
    VM vm;
    vm_init(&vm);

    // Create a chunk for a simple function: add 1 to arg
    Chunk func_chunk;
    init_chunk(&func_chunk);
    add_constant(&func_chunk, (Value){.type = VAL_NUMBER, .as.number = 1});
    write_instruction(&func_chunk, make_instruction(OP_CONSTANT, 0)); // push 1
    write_instruction(&func_chunk, make_instruction(OP_ADD, 0));
    write_instruction(&func_chunk, make_instruction(OP_RETURN, 0));

    // Create a main chunk
    Chunk main_chunk;
    init_chunk(&main_chunk);
    add_constant(&main_chunk, (Value){.type = VAL_NUMBER, .as.number = 10});
    write_instruction(&main_chunk, make_instruction(OP_CONSTANT, 0)); // push 10 (arg)
    write_instruction(&main_chunk, make_instruction(OP_HALT, 0));      // Will be executed after the function returns

    // We will now manually set up the stack and call frames as if a CALL has already happened.
    // This allows us to test RETURN in isolation.

    vm.stack_top = vm.stack;
    push(&vm, (Value){.type = VAL_NUMBER, .as.number = 10}); // Push arg

    // Set up the "calling" frame (main)
    vm.frames[0].chunk = &main_chunk;
    vm.frames[0].ip = main_chunk.code.code + 1; // IP is *after* the "call" (the push)
    vm.frames[0].slots = vm.stack;
    
    // Set up the "callee" frame (the function we are in)
    vm.frames[1].chunk = &func_chunk;
    vm.frames[1].ip = func_chunk.code.code; // Start at beginning of function
    vm.frames[1].slots = vm.stack_top - 1; // Function's frame starts at the arg
    vm.frame_count = 2;

    vm_run(&vm); // This will start executing from func_chunk

    // After return, stack top should be 11
    ASSERT_EQ(vm.stack_top - vm.stack, (size_t)1, "Stack should have one value, but has %zu");
    ASSERT_EQ(vm.stack[0].as.number, (int64_t)11, "Result should be 11, but was %lld");

    free_chunk(&func_chunk);
    free_chunk(&main_chunk);
}

TEST(test_op_call) {
    VM vm;
    vm_init(&vm);

    // Function to be called
    Chunk func_chunk;
    init_chunk(&func_chunk);
    write_instruction(&func_chunk, make_instruction(OP_ADD, 0));
    write_instruction(&func_chunk, make_instruction(OP_RETURN, 0));
    Function func = { .chunk = &func_chunk };
    
    // Main chunk
    Chunk main_chunk;
    init_chunk(&main_chunk);
    add_constant(&main_chunk, (Value){ .type = VAL_NUMBER, .as.number = 5 });
    add_constant(&main_chunk, (Value){ .type = VAL_NUMBER, .as.number = 10 });
    add_constant(&main_chunk, (Value){ .type = VAL_FUNCTION, .as.function = &func });

    write_instruction(&main_chunk, make_instruction(OP_CONSTANT, 2)); // push function
    write_instruction(&main_chunk, make_instruction(OP_CONSTANT, 0)); // push 5
    write_instruction(&main_chunk, make_instruction(OP_CONSTANT, 1)); // push 10
    write_instruction(&main_chunk, make_instruction(OP_CALL, 2));     // call with 2 args
    write_instruction(&main_chunk, make_instruction(OP_HALT, 0));
    
    // Set up initial frame
    CallFrame* frame = &vm.frames[vm.frame_count++];
    frame->chunk = &main_chunk;
    frame->ip = main_chunk.code.code;
    frame->slots = vm.stack;

    vm_run(&vm);

    ASSERT_EQ(vm.stack_top - vm.stack, (size_t)1, "Stack should have one value (actual: %ld)");
    ASSERT_EQ(vm.stack[0].as.number, (int64_t)15, "Result should be 15 (actual: %lld)");
    
    free_chunk(&func_chunk);
    free_chunk(&main_chunk);
}

int main(void) {
    RUN_TEST(test_simple_addition);
    RUN_TEST(test_jmp_if_false);
    RUN_TEST(test_unconditional_jump);
    RUN_TEST(test_function_call);
    RUN_TEST(test_op_call);
    printf("✔︎ All execution tests passed.\n");
    return 0;
} 