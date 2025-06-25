#ifndef VM_H
#define VM_H
#include "common.h"
#include "value.h"
#include "chunk.h"

struct Chunk; // Forward declaration
typedef uint64_t Instruction;

#define MAX_FRAMES 64
#define VM_INIT_STACK_SIZE 256

typedef struct {
    struct Chunk *chunk;
    Instruction* ip;
    Value* slots;
} CallFrame;

typedef struct {
    CallFrame frames[MAX_FRAMES];
    int frame_count;

    Value stack[VM_INIT_STACK_SIZE];
    Value *stack_top;
} VM;

void vm_init(VM *vm);
void vm_free(VM *vm);
void vm_run(VM *vm);
void push(VM *vm, Value value);
Value pop(VM *vm);

static const int DEBUG_INFO = 0;

#endif
