#include "../opcode.h"
#include "test_macros.h"

TEST(test_instruction_roundtrip) {
    const uint8_t opcode = 0x2A;
    const uint64_t operand = 0x1234567890ABCDULL;

    const Instruction inst = make_instruction(opcode, operand);

    ASSERT_EQ(get_opcode(inst), opcode, "%02X");
    ASSERT_EQ(get_operand(inst), operand & OPERAND_MASK, "%014llX");
}

TEST(test_operand_masking) {
    const uint64_t operand = 0xFFFFFFFFFFFFFFFFULL;
    const Instruction inst = make_instruction(0x01, operand);

    ASSERT_EQ(get_operand(inst), operand & OPERAND_MASK, "%014llX");
}

int main(void) {
    RUN_TEST(test_instruction_roundtrip);
    RUN_TEST(test_operand_masking);

    printf("✔︎ All opcode tests passed.\n");
    return 0;
} 