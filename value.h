#ifndef KAPPAVM_VALUE_H
#define KAPPAVM_VALUE_H
#include "common.h"

struct Chunk; // Forward-declare

typedef enum {
    VAL_NULL, VAL_NUMBER, VAL_STRING, VAL_LIST, VAL_OBJECT, VAL_FUNCTION
} ValueType;

typedef struct Value Value;
typedef struct Object Object;
typedef struct Function Function;

typedef struct {
    size_t length;
    Value *items;
    size_t capacity;
} List;

struct Object{
    size_t count;
    char **keys;
    Value *values;
    size_t capacity;
    Object *prototype;
};

struct Value {
    ValueType type;
    union {
        double fp_number;
        char *string;
        int64_t number;
        List *list;
        Object *object;
        Function *function;
    } as;
};

struct Function {
    struct Chunk* chunk;
    // We can add more here later, like arity, name for debugging, etc.
};

#endif //KAPPAVM_VALUE_H 