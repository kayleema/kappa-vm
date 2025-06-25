#ifndef KAPPAVM_ASSEMBLER_H
#define KAPPAVM_ASSEMBLER_H

#include "chunk.h"

Chunk assemble_chunk_from_string(const char *src);
int assemble_chunk_from_file(const char* in_filename, const char* out_filename);

#endif //KAPPAVM_ASSEMBLER_H 