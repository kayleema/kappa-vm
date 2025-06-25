#include "assembler.h"
#include "chunk.h"
#include "vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [--dis | --assemble <in> <out> | <file>]\n", argv[0]);
        return 1;
    }
    if (argc == 4 && strcmp(argv[1], "--assemble") == 0) {
        if (assemble_chunk_from_file(argv[2], argv[3]) != 0) {
            fprintf(stderr, "Failed to assemble %s to %s\n", argv[2], argv[3]);
            return 1;
        }
        return 0;
    }
    int disassemble = 0;
    const char *filename = NULL;
    if (argc == 3 && strcmp(argv[1], "--dis") == 0) {
        disassemble = 1;
        filename = argv[2];
    } else if (argc == 2) {
        filename = argv[1];
    } else {
        fprintf(stderr, "Usage: %s [--dis | --assemble <in> <out> | <file>]\n", argv[0]);
        return 1;
    }
    Chunk chunk;
    init_chunk(&chunk);
    if (load_chunk(&chunk, filename) != 0) {
        fprintf(stderr, "Failed to load bytecode file: %s\n", filename);
        return 2;
    }
    if (disassemble) {
        disassemble_chunk(&chunk, stdout);
        free_chunk(&chunk);
        return 0;
    }
    VM vm;
    vm_init(&vm);
    
    CallFrame* frame = &vm.frames[vm.frame_count++];
    frame->chunk = &chunk;
    frame->ip = chunk.code.code;
    frame->slots = vm.stack;

    vm_run(&vm);

    if (vm.stack_top > vm.stack) {
        const Value result = *(vm.stack_top - 1);
        if (result.type == VAL_NUMBER) {
            printf("%lld\n", result.as.number);
        } else {
            printf("[non-number result]\n");
        }
    } else {
        printf("[no result]\n");
    }
    vm_free(&vm);
    free_chunk(&chunk);
    return 0;
}