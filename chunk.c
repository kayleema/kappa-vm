#include "chunk.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define KAPPA_MAGIC "KBC0"
#define KAPPA_VERSION 1

void init_chunk(Chunk* chunk) {
    chunk->code.count = 0;
    chunk->code.capacity = 0;
    chunk->code.code = NULL;
    chunk->constants.count = 0;
    chunk->constants.capacity = 0;
    chunk->constants.values = NULL;
}

void free_chunk(Chunk* chunk) {
    free(chunk->code.code);
    free(chunk->constants.values);
    init_chunk(chunk);
}

size_t add_constant(Chunk* chunk, Value value) {
    if (chunk->constants.capacity < chunk->constants.count + 1) {
        size_t old_capacity = chunk->constants.capacity;
        chunk->constants.capacity = old_capacity < 8 ? 8 : old_capacity * 2;
        chunk->constants.values = realloc(chunk->constants.values, sizeof(Value) * chunk->constants.capacity);
    }

    chunk->constants.values[chunk->constants.count] = value;
    return chunk->constants.count++;
}

void write_instruction(Chunk* chunk, Instruction instruction) {
    if (chunk->code.capacity < chunk->code.count + 1) {
        size_t old_capacity = chunk->code.capacity;
        chunk->code.capacity = old_capacity < 8 ? 8 : old_capacity * 2;
        chunk->code.code = realloc(chunk->code.code, sizeof(Instruction) * chunk->code.capacity);
    }

    chunk->code.code[chunk->code.count] = instruction;
    chunk->code.count++;
}

// Internal helper for recursive saving
static int save_chunk_internal(const Chunk* chunk, FILE* f) {
    // Write counts
    uint64_t const_count = chunk->constants.count;
    uint64_t code_count = chunk->code.count;
    fwrite(&const_count, sizeof(uint64_t), 1, f);
    fwrite(&code_count, sizeof(uint64_t), 1, f);
    // Write constants
    for (size_t i = 0; i < chunk->constants.count; i++) {
        uint8_t type = chunk->constants.values[i].type;
        fwrite(&type, 1, 1, f);
        if (type == VAL_NUMBER) {
            int64_t num = chunk->constants.values[i].as.number;
            fwrite(&num, sizeof(int64_t), 1, f);
        } else if (type == VAL_FUNCTION) {
            Function* fn = chunk->constants.values[i].as.function;
            if (!fn || !fn->chunk) return -3;
            int res = save_chunk_internal(fn->chunk, f);
            if (res != 0) return res;
        } else {
            return -2;
        }
    }
    // Write instructions
    fwrite(chunk->code.code, sizeof(Instruction), chunk->code.count, f);
    return 0;
}

int save_chunk(const Chunk* chunk, const char* filename) {
    FILE* f = fopen(filename, "wb");
    if (!f) return -1;
    // Write header
    fwrite(KAPPA_MAGIC, 1, 4, f);
    uint32_t version = KAPPA_VERSION;
    fwrite(&version, sizeof(uint32_t), 1, f);
    int res = save_chunk_internal(chunk, f);
    fclose(f);
    return res;
}

// Internal helper for recursive loading
static int load_chunk_internal(Chunk* chunk, FILE* f, const int depth) {
    if (depth > 1000) {
        fprintf(stderr, "Maximum chunk depth exceeded\n");
        return -5;
    }

    uint64_t const_count = 0, code_count = 0;
    fread(&const_count, sizeof(uint64_t), 1, f);
    fread(&code_count, sizeof(uint64_t), 1, f);
    // Read constants
    for (size_t i = 0; i < const_count; i++) {
        uint8_t type = 0;
        fread(&type, 1, 1, f);
        if (type == VAL_NUMBER) {
            int64_t num = 0;
            fread(&num, sizeof(int64_t), 1, f);
            add_constant(chunk, (Value){.type = VAL_NUMBER, .as.number = num});
        } else if (type == VAL_FUNCTION) {
            Chunk* fn_chunk = malloc(sizeof(Chunk));
            init_chunk(fn_chunk);
            const int res = load_chunk_internal(fn_chunk, f, depth + 1);
            if (res != 0) {
                free_chunk(fn_chunk);
                free(fn_chunk);
                return res;
            }
            Function* fn = malloc(sizeof(Function));
            fn->chunk = fn_chunk;
            add_constant(chunk, (Value){.type = VAL_FUNCTION, .as.function = fn});
        } else {
            return -4;
        }
    }
    // Read instructions
    for (size_t i = 0; i < code_count; i++) {
        Instruction inst = 0;
        fread(&inst, sizeof(Instruction), 1, f);
        write_instruction(chunk, inst);
    }
    return 0;
}

int load_chunk(Chunk* chunk, const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) return -1;
    char magic[5] = {0};
    fread(magic, 1, 4, f);
    if (strcmp(magic, KAPPA_MAGIC) != 0) { fclose(f); return -2; }
    uint32_t version = 0;
    fread(&version, sizeof(uint32_t), 1, f);
    if (version != KAPPA_VERSION) { fclose(f); return -3; }
    const int res = load_chunk_internal(chunk, f, 0);
    fclose(f);
    return res;
}

static void disassemble_chunk_with_indent(const Chunk* chunk, FILE* out, int indent);

void disassemble_chunk(const Chunk* chunk, FILE* out) {
    disassemble_chunk_with_indent(chunk, out, 0);
}

static void print_indent(FILE* out, int indent) {
    for (int i = 0; i < indent; i++) fputc(' ', out);
}

static void disassemble_chunk_with_indent(const Chunk* chunk, FILE* out, int indent) {
    print_indent(out, indent);
    fprintf(out, "== constants ==\n");
    for (size_t i = 0; i < chunk->constants.count; i++) {
        Value v = chunk->constants.values[i];
        print_indent(out, indent);
        if (v.type == VAL_NUMBER) {
            fprintf(out, "  %zu: number %lld\n", i, (long long)v.as.number);
        } else if (v.type == VAL_FUNCTION) {
            fprintf(out, "  %zu: function <#%p>\n", i, (void*)v.as.function);
            print_indent(out, indent);
            fprintf(out, "  -- function constant %zu disassembly --\n", i);
            if (v.as.function && v.as.function->chunk) {
                disassemble_chunk_with_indent(v.as.function->chunk, out, indent + 4);
            } else {
                print_indent(out, indent + 4);
                fprintf(out, "<null function chunk>\n");
            }
        } else {
            fprintf(out, "  %zu: [unknown type]\n", i);
        }
    }
    print_indent(out, indent);
    fprintf(out, "== code ==\n");
    for (size_t i = 0; i < chunk->code.count; i++) {
        Instruction inst = chunk->code.code[i];
        uint8_t opcode = get_opcode(inst);
        uint64_t operand = get_operand(inst);
        print_indent(out, indent);
        switch (opcode) {
            case OP_CONSTANT:
                fprintf(out, "  %zu: OP_CONSTANT %llu\n", i, (unsigned long long)operand);
                break;
            case OP_ADD:
                fprintf(out, "  %zu: OP_ADD %llu\n", i, (unsigned long long)operand);
                break;
            case OP_HALT:
                fprintf(out, "  %zu: OP_HALT %llu\n", i, (unsigned long long)operand);
                break;
            case OP_RETURN:
                fprintf(out, "  %zu: OP_RETURN %llu\n", i, (unsigned long long)operand);
                break;
            case OP_JMP:
                fprintf(out, "  %zu: OP_JMP %llu\n", i, (unsigned long long)operand);
                break;
            case OP_JMP_IF_FALSE:
                fprintf(out, "  %zu: OP_JMP_IF_FALSE %llu\n", i, (unsigned long long)operand);
                break;
            case OP_CALL:
                fprintf(out, "  %zu: OP_CALL %llu\n", i, (unsigned long long)operand);
                break;
            default:
                fprintf(out, "  %zu: [unknown opcode %u] %llu\n", i, opcode, (unsigned long long)operand);
                break;
        }
    }
} 