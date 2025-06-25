#include "../chunk.h"
#include "test_macros.h"
#include <stdlib.h>
#include <string.h>

TEST(test_init_chunk) {
    Chunk chunk;
    init_chunk(&chunk);
    ASSERT_EQ(chunk.code.count, (size_t)0, "%zu");
    ASSERT_EQ(chunk.code.capacity, (size_t)0, "%zu");
    ASSERT_EQ(chunk.code.code, NULL, "%p");
    ASSERT_EQ(chunk.constants.count, (size_t)0, "%zu");
    ASSERT_EQ(chunk.constants.capacity, (size_t)0, "%zu");
    ASSERT_EQ(chunk.constants.values, NULL, "%p");
    free_chunk(&chunk);
}

TEST(test_write_and_grow_chunk) {
    Chunk chunk;
    init_chunk(&chunk);

    // Write enough instructions to force a resize
    for (int i = 0; i < 10; i++) {
        write_instruction(&chunk, make_instruction(OP_HALT, i));
    }

    ASSERT_EQ(chunk.code.count, (size_t)10, "%zu");
    ASSERT_GT(chunk.code.capacity, (size_t)10, "%zu");

    for (int i = 0; i < 10; i++) {
        ASSERT_EQ(get_opcode(chunk.code.code[i]), OP_HALT, "%d");
        ASSERT_EQ(get_operand(chunk.code.code[i]), (uint64_t)i, "%llu");
    }

    // Write enough constants to force a resize
    for (int i = 0; i < 10; i++) {
        add_constant(&chunk, (Value){.type = VAL_NUMBER, .as.number = i});
    }

    ASSERT_EQ(chunk.constants.count, (size_t)10, "%zu");
    ASSERT_GT(chunk.constants.capacity, (size_t)10, "%zu");
    for (int i = 0; i < 10; i++) {
        ASSERT_EQ(chunk.constants.values[i].type, VAL_NUMBER, "%d");
        ASSERT_EQ(chunk.constants.values[i].as.number, (int64_t)i, "%lld");
    }

    free_chunk(&chunk);
}

TEST(test_chunk_serialize_deserialize) {
    Chunk chunk;
    init_chunk(&chunk);

    // Populate chunk
    for (int i = 0; i < 5; i++) {
        add_constant(&chunk, (Value){.type = VAL_NUMBER, .as.number = i * 10});
        write_instruction(&chunk, make_instruction(OP_CONSTANT, i));
    }
    write_instruction(&chunk, make_instruction(OP_HALT, 0));

    // Save to file
    const char *filename = "test_chunk.kbc";
    int save_result = save_chunk(&chunk, filename);
    ASSERT_EQ(save_result, 0, "%d");

    // Load from file
    Chunk loaded;
    init_chunk(&loaded);
    int load_result = load_chunk(&loaded, filename);
    ASSERT_EQ(load_result, 0, "%d");

    // Compare constants
    ASSERT_EQ(loaded.constants.count, chunk.constants.count, "%zu");
    for (size_t i = 0; i < chunk.constants.count; i++) {
        ASSERT_EQ(loaded.constants.values[i].type, chunk.constants.values[i].type, "%d");
        ASSERT_EQ(loaded.constants.values[i].as.number, chunk.constants.values[i].as.number, "%lld");
    }
    // Compare code
    ASSERT_EQ(loaded.code.count, chunk.code.count, "%zu");
    for (size_t i = 0; i < chunk.code.count; i++) {
        ASSERT_EQ(loaded.code.code[i], chunk.code.code[i], "%llx");
    }

    free_chunk(&chunk);
    free_chunk(&loaded);
    remove(filename);
}

void disassemble_chunk(const Chunk* chunk, FILE* out);

TEST(test_disassemble_chunk) {
    Chunk chunk;
    init_chunk(&chunk);
    size_t c1 = add_constant(&chunk, (Value){.type = VAL_NUMBER, .as.number = 42});
    write_instruction(&chunk, make_instruction(OP_CONSTANT, c1));
    write_instruction(&chunk, make_instruction(OP_HALT, 0));

    char *buf = NULL;
    size_t buflen = 0;
    FILE *mem = open_memstream(&buf, &buflen);
    disassemble_chunk(&chunk, mem);
    fclose(mem);

    const char *expected =
        "== constants ==\n"
        "  0: number 42\n"
        "== code ==\n"
        "  0: OP_CONSTANT 0\n"
        "  1: OP_HALT 0\n";
    if (strcmp(buf, expected) != 0) {
        fprintf(stderr, "Disassembly output did not match expected.\nGot:\n%s\nExpected:\n%s\n", buf, expected);
        free(buf);
        free_chunk(&chunk);
        exit(1);
    }
    free(buf);
    free_chunk(&chunk);
}

TEST(test_disassemble_chunk_with_add) {
    Chunk chunk;
    init_chunk(&chunk);
    size_t c1 = add_constant(&chunk, (Value){.type = VAL_NUMBER, .as.number = 1});
    size_t c2 = add_constant(&chunk, (Value){.type = VAL_NUMBER, .as.number = 2});
    write_instruction(&chunk, make_instruction(OP_CONSTANT, c1));
    write_instruction(&chunk, make_instruction(OP_CONSTANT, c2));
    write_instruction(&chunk, make_instruction(OP_ADD, 0));
    write_instruction(&chunk, make_instruction(OP_HALT, 0));

    char *buf = NULL;
    size_t buflen = 0;
    FILE *mem = open_memstream(&buf, &buflen);
    disassemble_chunk(&chunk, mem);
    fclose(mem);

    const char *expected =
        "== constants ==\n"
        "  0: number 1\n"
        "  1: number 2\n"
        "== code ==\n"
        "  0: OP_CONSTANT 0\n"
        "  1: OP_CONSTANT 1\n"
        "  2: OP_ADD 0\n"
        "  3: OP_HALT 0\n";
    if (strcmp(buf, expected) != 0) {
        fprintf(stderr, "Disassembly output did not match expected.\nGot:\n%s\nExpected:\n%s\n", buf, expected);
        free(buf);
        free_chunk(&chunk);
        exit(1);
    }
    free(buf);
    free_chunk(&chunk);
}

Chunk assemble_chunk_from_string(const char *src);

TEST(test_assemble_chunk_from_string) {
    const char *src =
        "CONSTANT 42\n"
        "CONSTANT 17\n"
        "ADD\n"
        "HALT\n";
    Chunk chunk = assemble_chunk_from_string(src);
    ASSERT_EQ(chunk.constants.count, (size_t)2, "%zu");
    ASSERT_EQ(chunk.constants.values[0].type, VAL_NUMBER, "%d");
    ASSERT_EQ(chunk.constants.values[0].as.number, (int64_t)42, "%lld");
    ASSERT_EQ(chunk.constants.values[1].type, VAL_NUMBER, "%d");
    ASSERT_EQ(chunk.constants.values[1].as.number, (int64_t)17, "%lld");
    ASSERT_EQ(chunk.code.count, (size_t)4, "%zu");
    ASSERT_EQ(get_opcode(chunk.code.code[0]), OP_CONSTANT, "%d");
    ASSERT_EQ(get_operand(chunk.code.code[0]), (uint64_t)0, "%llu");
    ASSERT_EQ(get_opcode(chunk.code.code[1]), OP_CONSTANT, "%d");
    ASSERT_EQ(get_operand(chunk.code.code[1]), (uint64_t)1, "%llu");
    ASSERT_EQ(get_opcode(chunk.code.code[2]), OP_ADD, "%d");
    ASSERT_EQ(get_operand(chunk.code.code[2]), (uint64_t)0, "%llu");
    ASSERT_EQ(get_opcode(chunk.code.code[3]), OP_HALT, "%d");
    ASSERT_EQ(get_operand(chunk.code.code[3]), (uint64_t)0, "%llu");
    free_chunk(&chunk);
}

TEST(test_chunk_save_load_function_constant) {
    // Create a function chunk (returns 42)
    Chunk* func_chunk = malloc(sizeof(Chunk));
    init_chunk(func_chunk);
    size_t c1 = add_constant(func_chunk, (Value){.type = VAL_NUMBER, .as.number = 42});
    write_instruction(func_chunk, make_instruction(OP_CONSTANT, c1));
    write_instruction(func_chunk, make_instruction(OP_RETURN, 0));
    Function* func = malloc(sizeof(Function));
    func->chunk = func_chunk;

    // Create a main chunk and add the function as a constant
    Chunk main_chunk;
    init_chunk(&main_chunk);
    add_constant(&main_chunk, (Value){.type = VAL_FUNCTION, .as.function = func});
    write_instruction(&main_chunk, make_instruction(OP_CONSTANT, 0)); // push function
    write_instruction(&main_chunk, make_instruction(OP_HALT, 0));

    // Save to file
    const char *filename = "test_func_const.kbc";
    int save_result = save_chunk(&main_chunk, filename);
    ASSERT_EQ(save_result, 0, "%d");

    // Load from file
    Chunk loaded;
    init_chunk(&loaded);
    int load_result = load_chunk(&loaded, filename);
    ASSERT_EQ(load_result, 0, "%d");

    // Check that the loaded chunk has a function constant
    ASSERT_EQ(loaded.constants.count, (size_t)1, "%zu");
    Value loaded_func_val = loaded.constants.values[0];
    ASSERT_EQ(loaded_func_val.type, VAL_FUNCTION, "%d");
    ASSERT_NE(loaded_func_val.as.function, NULL, "%p");
    ASSERT_NE(loaded_func_val.as.function->chunk, NULL, "%p");
    // Check that the nested chunk has the right constant and code
    Chunk* loaded_func_chunk = loaded_func_val.as.function->chunk;
    ASSERT_EQ(loaded_func_chunk->constants.count, (size_t)1, "%zu");
    ASSERT_EQ(loaded_func_chunk->constants.values[0].as.number, (int64_t)42, "%lld");
    ASSERT_EQ(loaded_func_chunk->code.count, (size_t)2, "%zu");
    ASSERT_EQ(get_opcode(loaded_func_chunk->code.code[0]), OP_CONSTANT, "%d");
    ASSERT_EQ(get_opcode(loaded_func_chunk->code.code[1]), OP_RETURN, "%d");

    // Cleanup
    free_chunk(&main_chunk);
    free_chunk(func_chunk);
    free(func);
    free_chunk(&loaded);
    free(loaded_func_chunk);
    free(loaded_func_val.as.function);
    remove(filename);
}

TEST(test_disassemble_chunk_with_function_constant) {
    // Create a function chunk (returns 99)
    Chunk* func_chunk = malloc(sizeof(Chunk));
    init_chunk(func_chunk);
    size_t c1 = add_constant(func_chunk, (Value){.type = VAL_NUMBER, .as.number = 99});
    write_instruction(func_chunk, make_instruction(OP_CONSTANT, c1));
    write_instruction(func_chunk, make_instruction(OP_RETURN, 0));
    Function* func = malloc(sizeof(Function));
    func->chunk = func_chunk;

    // Create a main chunk and add the function as a constant
    Chunk main_chunk;
    init_chunk(&main_chunk);
    add_constant(&main_chunk, (Value){.type = VAL_FUNCTION, .as.function = func});
    write_instruction(&main_chunk, make_instruction(OP_CONSTANT, 0)); // push function
    write_instruction(&main_chunk, make_instruction(OP_HALT, 0));

    // Disassemble to a memory buffer
    char *buf = NULL;
    size_t buflen = 0;
    FILE *mem = open_memstream(&buf, &buflen);
    disassemble_chunk(&main_chunk, mem);
    fclose(mem);

    // Check that the output contains the recursive function disassembly
    const char *expected_substring = "-- function constant 0 disassembly --";
    if (!strstr(buf, expected_substring)) {
        fprintf(stderr, "Disassembly output did not include function constant disassembly.\nGot:\n%s\n", buf);
        free(buf);
        free_chunk(&main_chunk);
        free_chunk(func_chunk);
        free(func);
        exit(1);
    }
    // Optionally, check for the nested constant and code
    if (!strstr(buf, "number 99") || !strstr(buf, "OP_RETURN")) {
        fprintf(stderr, "Disassembly output did not include nested function chunk details.\nGot:\n%s\n", buf);
        free(buf);
        free_chunk(&main_chunk);
        free_chunk(func_chunk);
        free(func);
        exit(1);
    }

    free(buf);
    free_chunk(&main_chunk);
    free_chunk(func_chunk);
    free(func);
}

int main(void) {
    RUN_TEST(test_init_chunk);
    RUN_TEST(test_write_and_grow_chunk);
    RUN_TEST(test_chunk_serialize_deserialize);
    RUN_TEST(test_disassemble_chunk);
    RUN_TEST(test_disassemble_chunk_with_add);
    RUN_TEST(test_assemble_chunk_from_string);
    RUN_TEST(test_chunk_save_load_function_constant);
    RUN_TEST(test_disassemble_chunk_with_function_constant);
    printf("✔︎ All chunk tests passed.\n");
    return 0;
} 