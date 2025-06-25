#include "vm.h"
#include "chunk.h"
#include "opcode.h"
#include <stdio.h>

void push(VM *vm, Value value) {
    *vm->stack_top = value;
    vm->stack_top++;
}

Value pop(VM *vm) {
    vm->stack_top--;
    return *vm->stack_top;
}

static Value peek(VM *vm, int distance) {
    return vm->stack_top[-1 - distance];
}

static int is_falsey(Value value) {
    return value.type == VAL_NULL || (value.type == VAL_NUMBER && value.as.number == 0);
}

void vm_init(VM *vm) {
    vm->frame_count = 0;
    vm->stack_top = vm->stack;
}

void vm_free(VM *vm) {
}

void vm_run(VM *vm) {
    CallFrame *frame = &vm->frames[vm->frame_count - 1];

    while (1) {
        if (DEBUG_INFO) {
            int line_number = frame->ip - frame->chunk->code.code;
            fprintf(stderr, "[%d] ", line_number);
            // print stack
            for (int i = 0; i < vm->stack_top - vm->stack; i++) {
                fprintf(stderr, "%lld ", vm->stack[i].as.number);
            }
            fprintf(stderr, "\n");
        }

        Instruction instruction = *frame->ip++;
        switch (get_opcode(instruction)) {
            case OP_CONSTANT: {
                push(vm, frame->chunk->constants.values[get_operand(instruction)]);
                break;
            }
            case OP_ADD: {
                Value b = pop(vm);
                Value a = pop(vm);
                push(vm, (Value){.type = VAL_NUMBER, .as.number = a.as.number + b.as.number});
                break;
            }
            case OP_JMP: {
                frame->ip += (int16_t) get_operand(instruction);
                break;
            }
            case OP_JMP_IF_FALSE: {
                if (is_falsey(pop(vm))) {
                    frame->ip += (int16_t) get_operand(instruction);
                }
                break;
            }
            case OP_CALL: {
                uint8_t arg_count = get_operand(instruction);
                Value callee = peek(vm, arg_count);

                if (callee.type != VAL_FUNCTION) {
                    fprintf(stderr, "RuntimeError: Can only call functions.\n");
                    return;
                }

                Function *function = callee.as.function;

                if (vm->frame_count == MAX_FRAMES) {
                    fprintf(stderr, "RuntimeError: Stack overflow.\n");
                    return;
                }

                CallFrame *new_frame = &vm->frames[vm->frame_count++];
                new_frame->chunk = function->chunk;
                new_frame->ip = function->chunk->code.code;
                new_frame->slots = vm->stack_top - arg_count - 1;

                frame = new_frame;
                break;
            }
            case OP_RETURN: {
                Value return_value = pop(vm);
                vm->frame_count--;
                if (vm->frame_count == 0) {
                    pop(vm);
                    return;
                }
                frame = &vm->frames[vm->frame_count - 1];
                vm->stack_top = frame->slots;
                push(vm, return_value);
                break;
            }
            case OP_HALT: {
                return;
            }
        }
    }
}
