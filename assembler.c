#include "assembler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    char name[256];
    size_t address;
} Label;

typedef struct {
    char label_name[256];
    size_t instruction_address;
    size_t line_number; // For error reporting
} UnresolvedJump;

Chunk assemble_chunk_from_string(const char *src) {
    Chunk chunk;
    init_chunk(&chunk);
    Label labels[256];
    size_t label_count = 0;
    UnresolvedJump unresolved[256];
    size_t unresolved_count = 0;

    char *src_copy = strdup(src);
    char *line_saveptr;
    char *line = strtok_r(src_copy, "\n", &line_saveptr);
    size_t instruction_count = 0;
    size_t line_num = 0;

    // First pass: find labels
    while (line != NULL) {
        line_num++;
        char *trimmed_line = line;
        while (isspace(*trimmed_line)) trimmed_line++;

        char *colon = strchr(trimmed_line, ':');
        if (colon != NULL && *(colon + 1) == '\0') {
            *colon = '\0'; // Null-terminate the label name
            strcpy(labels[label_count].name, trimmed_line);
            labels[label_count].address = instruction_count;
            label_count++;
        } else if (strlen(trimmed_line) > 0) {
            instruction_count++;
        }
        line = strtok_r(NULL, "\n", &line_saveptr);
    }
    free(src_copy);

    // Second pass: assemble
    src_copy = strdup(src);
    line = strtok_r(src_copy, "\n", &line_saveptr);
    instruction_count = 0;
    line_num = 0;
    while (line != NULL) {
        line_num++;
        char *colon = strchr(line, ':');
        if (colon != NULL && *(colon + 1) == '\0') {
             line = strtok_r(NULL, "\n", &line_saveptr);
             continue;
        }

        char *opcode_saveptr;
        char *opcode_str = strtok_r(line, " \t", &opcode_saveptr);

        if (opcode_str) {
             if (strcasecmp(opcode_str, "CONSTANT") == 0) {
                char *operand_str = strtok_r(NULL, " \t", &opcode_saveptr);
                if (operand_str) {
                    long long num = atoll(operand_str);
                    size_t const_idx = add_constant(&chunk, (Value){.type = VAL_NUMBER, .as.number = num});
                    write_instruction(&chunk, make_instruction(OP_CONSTANT, const_idx));
                }
             } else if (strcasecmp(opcode_str, "ADD") == 0) {
                write_instruction(&chunk, make_instruction(OP_ADD, 0));
             } else if (strcasecmp(opcode_str, "HALT") == 0) {
                write_instruction(&chunk, make_instruction(OP_HALT, 0));
             } else if (strcasecmp(opcode_str, "JMP") == 0 || strcasecmp(opcode_str, "JMP_IF_FALSE") == 0) {
                 char *label_name = strtok_r(NULL, " \t", &opcode_saveptr);
                 strcpy(unresolved[unresolved_count].label_name, label_name);
                 unresolved[unresolved_count].instruction_address = instruction_count;
                 unresolved[unresolved_count].line_number = line_num;
                 unresolved_count++;
                 write_instruction(&chunk, make_instruction(strcasecmp(opcode_str, "JMP") == 0 ? OP_JMP : OP_JMP_IF_FALSE, 0)); // Placeholder operand
             }
        }
        instruction_count++;
        line = strtok_r(NULL, "\n", &line_saveptr);
    }
    free(src_copy);
    
    // Resolve jumps
    for (size_t i = 0; i < unresolved_count; i++) {
        int found = 0;
        for (size_t j = 0; j < label_count; j++) {
            if (strcmp(unresolved[i].label_name, labels[j].name) == 0) {
                int64_t offset = (int64_t)labels[j].address - ((int64_t)unresolved[i].instruction_address + 1);
                uint8_t opcode = get_opcode(chunk.code.code[unresolved[i].instruction_address]);
                chunk.code.code[unresolved[i].instruction_address] = make_instruction(opcode, (uint64_t)offset);
                found = 1;
                break;
            }
        }
        if (!found) { /* handle error */ }
    }
    return chunk;
}

int assemble_chunk_from_file(const char* in_filename, const char* out_filename) {
    FILE *f_in = fopen(in_filename, "r");
    if (!f_in) return -1;

    fseek(f_in, 0, SEEK_END);
    long fsize = ftell(f_in);
    fseek(f_in, 0, SEEK_SET);

    char *string = malloc(fsize + 1);
    fread(string, 1, fsize, f_in);
    fclose(f_in);

    string[fsize] = 0;

    Chunk chunk = assemble_chunk_from_string(string);
    free(string);

    int result = save_chunk(&chunk, out_filename);
    free_chunk(&chunk);

    return result;
} 