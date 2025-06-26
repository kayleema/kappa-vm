#include "../assembler.h"
#include "../chunk.h"
#include "test_macros.h"
#include <string.h>

TEST(test_assemble_labels_and_jumps) {
    const char *src =
        "  CONSTANT 1\n"         // 0
        "  JMP_IF_FALSE end\n"     // 1
        "  CONSTANT 10\n"        // 2
        "  JMP finish\n"         // 3
        "end:\n"                 // (address 4)
        "  CONSTANT 20\n"        // 4
        "finish:\n"              // (address 5)
        "  ADD\n"                // 5
        "  HALT\n";              // 6

    Chunk chunk = assemble_chunk_from_string(src);

    // Check constants
    ASSERT_EQ(chunk.constants.count, (size_t)3, "%zu"); // 1, 10, 20

    // Check code
    ASSERT_EQ(chunk.code.count, (size_t)7, "%zu");
    ASSERT_EQ(get_opcode(chunk.code.code[1]), OP_JMP_IF_FALSE, "%d");
    ASSERT_EQ(get_operand(chunk.code.code[1]), (uint64_t)2, "JMP_IF_FALSE operand should be 2, but was %llu");

    ASSERT_EQ(get_opcode(chunk.code.code[3]), OP_JMP, "%d");
    ASSERT_EQ(get_operand(chunk.code.code[3]), (uint64_t)1, "JMP operand should be 1, but was %llu");

    free_chunk(&chunk);
}

TEST(test_assemble_call_and_return) {
    const char *src =
        "  CONSTANT 5\n"          // 0
        "  CONSTANT 10\n"         // 1  
        "  CALL 2\n"              // 2
        "  HALT\n"                // 3
        "  ADD\n"                 // 4
        "  RETURN\n";             // 5

    Chunk chunk = assemble_chunk_from_string(src);

    // Check constants
    ASSERT_EQ(chunk.constants.count, (size_t)2, "%zu"); // 5, 10

    // Check code
    ASSERT_EQ(chunk.code.count, (size_t)6, "%zu");
    
    // Check CALL instruction
    ASSERT_EQ(get_opcode(chunk.code.code[2]), OP_CALL, "%d");
    ASSERT_EQ(get_operand(chunk.code.code[2]), (uint64_t)2, "CALL operand should be 2, but was %llu");
    
    // Check RETURN instruction
    ASSERT_EQ(get_opcode(chunk.code.code[5]), OP_RETURN, "%d");
    ASSERT_EQ(get_operand(chunk.code.code[5]), (uint64_t)0, "RETURN operand should be 0, but was %llu");

    free_chunk(&chunk);
}

TEST(test_assemble_program) {
    const char *src =
        "  CONSTANT 5\n"
        "  CONSTANT 10\n"
        "  ADD\n"
        "  HALT\n";

    Program program = assemble_program_from_string(src);

    // Check that main chunk was created correctly
    ASSERT_EQ(program.main_chunk.constants.count, (size_t)2, "%zu");
    ASSERT_EQ(program.main_chunk.code.count, (size_t)4, "%zu");
    
    // Check that no functions were created (for now)
    ASSERT_EQ(program.function_count, (size_t)0, "%zu");

    free_program(&program);
}

TEST(test_assemble_function_definition) {
    const char *src =
        "FUNCTION add_func\n"
        "  ADD\n"
        "  RETURN\n"
        "ENDFUNCTION\n"
        "\n"
        "  CONSTANT 5\n"
        "  CONSTANT 10\n"
        "  CONSTANT add_func\n"
        "  CALL 2\n"
        "  HALT\n";

    Program program = assemble_program_from_string(src);

    // Check that one function was created
    ASSERT_EQ(program.function_count, (size_t)1, "%zu");
    ASSERT_EQ(strcmp(program.functions[0].name, "add_func"), 0, "%d");
    
    // Check function chunk
    ASSERT_EQ(program.functions[0].chunk->code.count, (size_t)2, "%zu"); // ADD, RETURN
    ASSERT_EQ(get_opcode(program.functions[0].chunk->code.code[0]), OP_ADD, "%d");
    ASSERT_EQ(get_opcode(program.functions[0].chunk->code.code[1]), OP_RETURN, "%d");
    
    // Check main chunk has function reference
    ASSERT_EQ(program.main_chunk.constants.count, (size_t)3, "%zu"); // 5, 10, function
    ASSERT_EQ(program.main_chunk.constants.values[2].type, VAL_FUNCTION, "%d");

    free_program(&program);
}

int main(void) {
    RUN_TEST(test_assemble_labels_and_jumps);
    RUN_TEST(test_assemble_call_and_return);
    RUN_TEST(test_assemble_program);
    RUN_TEST(test_assemble_function_definition);
    printf("✔︎ All assembler tests passed.\n");
    return 0;
} 