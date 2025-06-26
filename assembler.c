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
             } else if (strcasecmp(opcode_str, "CALL") == 0) {
                 char *operand_str = strtok_r(NULL, " \t", &opcode_saveptr);
                 if (operand_str) {
                     long long arg_count = atoll(operand_str);
                     write_instruction(&chunk, make_instruction(OP_CALL, (uint64_t)arg_count));
                 }
             } else if (strcasecmp(opcode_str, "RETURN") == 0) {
                 write_instruction(&chunk, make_instruction(OP_RETURN, 0));
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
    // For backward compatibility, use the new program assembler but only save main chunk
    return assemble_program_from_file(in_filename, out_filename);
}

int assemble_program_from_file(const char* in_filename, const char* out_filename) {
    FILE *f_in = fopen(in_filename, "r");
    if (!f_in) return -1;

    fseek(f_in, 0, SEEK_END);
    long fsize = ftell(f_in);
    fseek(f_in, 0, SEEK_SET);

    char *string = malloc(fsize + 1);
    fread(string, 1, fsize, f_in);
    fclose(f_in);

    string[fsize] = 0;

    Program program = assemble_program_from_string(string);
    free(string);

    // For now, only save the main chunk (functions are lost)
    // TODO: Implement proper program serialization format
    int result = save_chunk(&program.main_chunk, out_filename);
    free_program(&program);

    return result;
}

static void assemble_main_with_functions(Program* program, const char* main_code) {
    char *src_copy = strdup(main_code);
    char *line_saveptr;
    char *line = strtok_r(src_copy, "\n", &line_saveptr);
    
    while (line != NULL) {
        char *trimmed_line = line;
        while (isspace(*trimmed_line)) trimmed_line++;
        
        if (strlen(trimmed_line) == 0 || trimmed_line[0] == '#') {
            line = strtok_r(NULL, "\n", &line_saveptr);
            continue;
        }
        
        char *opcode_saveptr;
        char *opcode_str = strtok_r(trimmed_line, " \t", &opcode_saveptr);
        
        if (opcode_str) {
            if (strcasecmp(opcode_str, "CONSTANT") == 0) {
                char *operand_str = strtok_r(NULL, " \t", &opcode_saveptr);
                if (operand_str) {
                    // Check if operand is a function name
                    int is_function = 0;
                    size_t func_idx = 0;
                    
                    for (size_t i = 0; i < program->function_count; i++) {
                        if (strcmp(operand_str, program->functions[i].name) == 0) {
                            // Add function to constants if not already added
                            Value func_value;
                            func_value.type = VAL_FUNCTION;
                            func_value.as.function = program->functions[i].function;
                            func_idx = add_constant(&program->main_chunk, func_value);
                            is_function = 1;
                            break;
                        }
                    }
                    
                    if (!is_function) {
                        // Regular number constant
                        long long num = atoll(operand_str);
                        func_idx = add_constant(&program->main_chunk, (Value){.type = VAL_NUMBER, .as.number = num});
                    }
                    
                    write_instruction(&program->main_chunk, make_instruction(OP_CONSTANT, func_idx));
                }
            } else if (strcasecmp(opcode_str, "ADD") == 0) {
                write_instruction(&program->main_chunk, make_instruction(OP_ADD, 0));
            } else if (strcasecmp(opcode_str, "HALT") == 0) {
                write_instruction(&program->main_chunk, make_instruction(OP_HALT, 0));
            } else if (strcasecmp(opcode_str, "CALL") == 0) {
                char *operand_str = strtok_r(NULL, " \t", &opcode_saveptr);
                if (operand_str) {
                    long long arg_count = atoll(operand_str);
                    write_instruction(&program->main_chunk, make_instruction(OP_CALL, (uint64_t)arg_count));
                }
            } else if (strcasecmp(opcode_str, "RETURN") == 0) {
                write_instruction(&program->main_chunk, make_instruction(OP_RETURN, 0));
            }
            // TODO: Add other opcodes as needed
        }
        
        line = strtok_r(NULL, "\n", &line_saveptr);
    }
    
    free(src_copy);
}

static void add_function_to_program(Program* program, const char* name, const char* func_src) {
    if (program->function_count >= program->function_capacity) {
        size_t old_capacity = program->function_capacity;
        program->function_capacity = old_capacity < 8 ? 8 : old_capacity * 2;
        program->functions = realloc(program->functions, 
            sizeof(FunctionDef) * program->function_capacity);
    }
    
    FunctionDef* func_def = &program->functions[program->function_count];
    strcpy(func_def->name, name);
    
    // Create chunk for function
    func_def->chunk = malloc(sizeof(Chunk));
    *func_def->chunk = assemble_chunk_from_string(func_src);
    
    // Create Function struct
    func_def->function = malloc(sizeof(Function));
    func_def->function->chunk = func_def->chunk;
    
    program->function_count++;
}

Program assemble_program_from_string(const char *src) {
    Program program;
    init_chunk(&program.main_chunk);
    program.functions = NULL;
    program.function_count = 0;
    program.function_capacity = 0;
    
    char *src_copy = strdup(src);
    char *main_code = malloc(strlen(src) + 1);
    main_code[0] = '\0';
    
    char *line_saveptr;
    char *line = strtok_r(src_copy, "\n", &line_saveptr);
    
    int in_function = 0;
    char current_function_name[256];
    char *function_code = NULL;
    size_t function_code_size = 0;
    size_t function_code_capacity = 0;
    
    while (line != NULL) {
        char *trimmed_line = line;
        while (isspace(*trimmed_line)) trimmed_line++;
        
        if (strncasecmp(trimmed_line, "FUNCTION ", 9) == 0) {
            // Start of function definition
            char *func_name = trimmed_line + 9;
            while (isspace(*func_name)) func_name++;
            strcpy(current_function_name, func_name);
            
            in_function = 1;
            function_code_size = 0;
        } else if (strcasecmp(trimmed_line, "ENDFUNCTION") == 0) {
            // End of function definition
            if (in_function && function_code) {
                function_code[function_code_size] = '\0';
                add_function_to_program(&program, current_function_name, function_code);
            }
            in_function = 0;
        } else if (in_function) {
            // Add line to function code
            size_t line_len = strlen(line);
            if (function_code_size + line_len + 2 > function_code_capacity) {
                function_code_capacity = (function_code_size + line_len + 2) * 2;
                function_code = realloc(function_code, function_code_capacity);
            }
            
            if (function_code_size > 0) {
                function_code[function_code_size++] = '\n';
            }
            strcpy(function_code + function_code_size, line);
            function_code_size += line_len;
        } else {
            // Add line to main code
            strcat(main_code, line);
            strcat(main_code, "\n");
        }
        
        line = strtok_r(NULL, "\n", &line_saveptr);
    }
    
    // Process main code with custom assembler that knows about functions
    if (strlen(main_code) > 0) {
        assemble_main_with_functions(&program, main_code);
    }
    
    free(src_copy);
    free(main_code);
    if (function_code) free(function_code);
    
    return program;
}

void free_program(Program* program) {
    free_chunk(&program->main_chunk);
    
    for (size_t i = 0; i < program->function_count; i++) {
        free_chunk(program->functions[i].chunk);
        free(program->functions[i].chunk);
        free(program->functions[i].function);
    }
    
    if (program->functions) {
        free(program->functions);
    }
} 