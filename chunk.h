#ifndef KAPPAVM_CHUNK_H
#define KAPPAVM_CHUNK_H

#include <stdio.h>
#include "common.h"
#include "value.h"
#include "opcode.h"

typedef struct Chunk Chunk;

typedef struct {
    size_t count;
    size_t capacity;
    Value* values;
} ConstantPool;

typedef struct {
    size_t count;
    size_t capacity;
    Instruction* code;
    // TODO: Add line number information for debugging
} Code;

struct Chunk {
    Code code;
    ConstantPool constants;
};


void init_chunk(Chunk* chunk);
void free_chunk(Chunk* chunk);
size_t add_constant(Chunk* chunk, Value value);
void write_instruction(Chunk* chunk, Instruction instruction);
int save_chunk(const Chunk* chunk, const char* filename);
int load_chunk(Chunk* chunk, const char* filename);
void disassemble_chunk(const Chunk* chunk, FILE* out);

#endif //KAPPAVM_CHUNK_H 