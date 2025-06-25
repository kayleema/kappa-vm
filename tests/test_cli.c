#include "../assembler.h"
#include "../chunk.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int test_cli_execution() {
    // Create a chunk file for testing
    Chunk chunk;
    init_chunk(&chunk);
    size_t c1 = add_constant(&chunk, (Value){.type = VAL_NUMBER, .as.number = 2});
    size_t c2 = add_constant(&chunk, (Value){.type = VAL_NUMBER, .as.number = 3});
    write_instruction(&chunk, make_instruction(OP_CONSTANT, c1));
    write_instruction(&chunk, make_instruction(OP_CONSTANT, c2));
    write_instruction(&chunk, make_instruction(OP_ADD, 0));
    write_instruction(&chunk, make_instruction(OP_HALT, 0));
    const char *filename = "test_cli.kbc";
    int save_result = save_chunk(&chunk, filename);
    if (save_result != 0) {
        fprintf(stderr, "Failed to save chunk for CLI test\n");
        return 1;
    }
    free_chunk(&chunk);

    // Run kappavm on the chunk file and capture output
    FILE *fp = popen("./kappavm test_cli.kbc", "r");
    if (!fp) {
        fprintf(stderr, "Failed to run kappavm\n");
        remove(filename);
        return 2;
    }
    char buf[128] = {0};
    fgets(buf, sizeof(buf), fp);
    int status = pclose(fp);
    if (status != 0) {
        fprintf(stderr, "kappavm did not exit cleanly (exit code %d)\n", status);
        remove(filename);
        return 3;
    }
    // Check output
    if (atoi(buf) != 5) {
        fprintf(stderr, "kappavm output was not 5, got: %s\n", buf);
        remove(filename);
        return 4;
    }
    remove(filename);
    printf("✔︎ CLI execution test passed.\n");
    return 0;
}

static int test_cli_disassembly() {
    Chunk chunk;
    init_chunk(&chunk);
    size_t c1 = add_constant(&chunk, (Value){.type = VAL_NUMBER, .as.number = 7});
    write_instruction(&chunk, make_instruction(OP_CONSTANT, c1));
    write_instruction(&chunk, make_instruction(OP_HALT, 0));
    const char *filename = "test_cli.kbc";
    int save_result = save_chunk(&chunk, filename);
    if (save_result != 0) {
        fprintf(stderr, "Failed to save chunk for CLI disassembly test\n");
        free_chunk(&chunk);
        return 1;
    }
    free_chunk(&chunk);

    FILE *fp = popen("./kappavm --dis test_cli.kbc", "r");
    if (!fp) {
        fprintf(stderr, "Failed to run kappavm for disassembly\n");
        remove(filename);
        return 2;
    }
    char buf[1024] = {0};
    fread(buf, 1, sizeof(buf) - 1, fp);
    int status = pclose(fp);
    if (status != 0) {
        fprintf(stderr, "kappavm disassembly did not exit cleanly (exit code %d)\n", status);
        remove(filename);
        return 3;
    }
    const char *expected =
        "== constants ==\n"
        "  0: number 7\n"
        "== code ==\n"
        "  0: OP_CONSTANT 0\n"
        "  1: OP_HALT 0\n";
    if (strcmp(buf, expected) != 0) {
        fprintf(stderr, "kappavm disassembly output did not match expected.\nGot:\n%s\nExpected:\n%s\n", buf, expected);
        remove(filename);
        return 4;
    }
    remove(filename);
    printf("✔︎ CLI disassembly test passed.\n");
    return 0;
}

static int test_cli_assembly() {
    // Create test.asm
    const char *asm_filename = "test.asm";
    FILE *f = fopen(asm_filename, "w");
    if (!f) return 1;
    fprintf(f, "CONSTANT 123\nHALT\n");
    fclose(f);

    // Run assembler
    int exit_code = system("./kappavm --assemble test.asm test.kbc");
    if (exit_code != 0) {
        remove(asm_filename);
        return 2;
    }

    // Load and check result
    Chunk loaded;
    init_chunk(&loaded);
    load_chunk(&loaded, "test.kbc");
    if (loaded.constants.count != 1 || loaded.code.count != 2) {
        free_chunk(&loaded);
        return 3;
    }
    // (Could add more specific assertions here)
    free_chunk(&loaded);

    remove(asm_filename);
    remove("test.kbc");
    printf("✔︎ CLI assembly test passed.\n");
    return 0;
}

int main(void) {
    if (test_cli_execution() != 0) return 1;
    if (test_cli_disassembly() != 0) return 1;
    if (test_cli_assembly() != 0) return 1;
    printf("✔︎ All CLI tests passed.\n");
    return 0;
} 