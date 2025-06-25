#ifndef KAPPAVM_OPCODE_H
#define KAPPAVM_OPCODE_H

#include <stdint.h>

typedef enum {
    OP_CONSTANT,
    OP_ADD,
    OP_HALT,
    OP_JMP_IF_FALSE,
    OP_JMP,
    OP_CALL,
    OP_RETURN,
} OpCode;

typedef uint64_t Instruction;

#define OPERAND_MASK 0x00FFFFFFFFFFFFFFULL

static inline uint8_t get_opcode(const Instruction inst) {
    return inst >> 56;
}

static inline uint64_t get_operand(const Instruction inst) {
    return inst & OPERAND_MASK;
}

static inline Instruction make_instruction(const uint8_t opcode, const uint64_t operand) {
    return (uint64_t)opcode << 56 | operand & OPERAND_MASK;
}

#endif //KAPPAVM_OPCODE_H 