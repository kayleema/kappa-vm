#ifndef KAPPAVM_ASSEMBLER_H
#define KAPPAVM_ASSEMBLER_H

#include "chunk.h"
#include "value.h"

typedef struct {
    char name[256];
    Chunk* chunk;
    Function* function;
} FunctionDef;

typedef struct {
    Chunk main_chunk;
    FunctionDef* functions;
    size_t function_count;
    size_t function_capacity;
} Program;

Chunk assemble_chunk_from_string(const char *src);
Program assemble_program_from_string(const char *src);
int assemble_chunk_from_file(const char* in_filename, const char* out_filename);
int assemble_program_from_file(const char* in_filename, const char* out_filename);
void free_program(Program* program);

#endif //KAPPAVM_ASSEMBLER_H 