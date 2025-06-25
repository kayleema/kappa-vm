#include "../assembler.h"
#include "../chunk.h"
#include "test_macros.h"

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

int main(void) {
    RUN_TEST(test_assemble_labels_and_jumps);
    printf("✔︎ All assembler tests passed.\n");
    return 0;
} 