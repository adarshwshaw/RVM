#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#define STACK_SIZE 1024 * 4

#define SHORT_TO_BYTE(x) ((x)&0xff), (((x)&0xff00) >> 8)
#define INT_TO_BYTE(x) SHORT_TO_BYTE((x)&0xffff), SHORT_TO_BYTE((x) >> 16)
#define LONG_TO_BYTE(x) INT_TO_BYTE((x)&0xffffffff), INT_TO_BYTE((x) >> 32)
typedef struct RVM_Stack {
    int32_t sp;
    uint8_t stack[STACK_SIZE];
} RVM_Stack;

typedef struct RVM_Heap {
    int32_t capacity;
    uint8_t *heap;
} RVM_Heap;

int spush(RVM_Stack *stack, uint8_t *val, uint8_t bytes) {
    for (uint8_t i = 0; i < bytes; i++) {
        stack->stack[++stack->sp] = *val++;
    }
    return 0;
}

int spop(RVM_Stack *stack, uint8_t *res, uint8_t bytes) {
    stack->sp -= bytes;
    for (uint8_t i = 0; i < bytes; i++) {
        res[i] = stack->stack[stack->sp + i + 1];
    }
    return 0;
}

void sDebugPrint(RVM_Stack *stack) {
    printf("stack [");
    for (int i = stack->sp; i > -1; --i) {
        printf("%d, ", stack->stack[i]);
    }
    printf("]\n");
}

// suffix b -> 1 byte, w -> 4bytes dw-> 8bytes sw->2bytes
typedef enum RVM_Inst {
    NOP = 0,
    PUSHB,
    PUSHSW,
    PUSHW,
    PUSHDW,
    POPB,
    POPSW,
    POPW,
    POPDW,
    HALT,
    ADDB,
    ADDSW,
    ADDW,
    ADDDW,
    SUBB,
    SUBSW,
    SUBW,
    SUBDW,
    MULB,
    MULSW,
    MULW,
    MULDW,
    DIVB,
    DIVSW,
    DIVW,
    DIVDW
} RVM_Inst;

typedef enum MathOp {
    RVM_ADD,
    RVM_SUB,
    RVM_MUL,
    RVM_DIV
} MathOp;

int RVM_MathOp(RVM_Stack *stack, MathOp op, uint8_t bytes) {
    int64_t res = 0, temp = 0;
    spop(stack, (uint8_t *)&res, bytes);
    spop(stack, (uint8_t *)&temp, bytes);
    switch (op) {
    case RVM_ADD:
        res += temp;
        break;
    case RVM_SUB:
        res -= temp;
        break;
    case RVM_MUL:
        res *= temp;
        break;
    case RVM_DIV:
        res /= temp;
        break;
    }
    spush(stack, (uint8_t *)(&res), bytes);
    return 0;
}
int RVM_RunProgram(uint8_t *program, uint32_t program_len) {
    RVM_Stack stack = {.sp = -1, .stack = {0}};
    for (uint32_t i = 0; i < program_len; i++) {
        RVM_Inst inst = (RVM_Inst)program[i];
        switch (inst) {
        case ADDB: {
            RVM_MathOp(&stack, RVM_ADD, 1);
            break;
        }
        case ADDSW: {
            RVM_MathOp(&stack, RVM_ADD, 2);
            break;
        }
        case ADDW: {
            RVM_MathOp(&stack, RVM_ADD, 4);
            break;
        }
        case ADDDW: {
            RVM_MathOp(&stack, RVM_ADD, 8);
            break;
        }
        case SUBB: {
            RVM_MathOp(&stack, RVM_SUB, 1);
            break;
        }
        case SUBSW: {
            RVM_MathOp(&stack, RVM_SUB, 2);
            break;
        }
        case SUBW: {
            RVM_MathOp(&stack, RVM_SUB, 4);
            break;
        }
        case SUBDW: {
            RVM_MathOp(&stack, RVM_SUB, 8);
            break;
        }
        case MULB: {
            RVM_MathOp(&stack, RVM_MUL, 1);
            break;
        }
        case MULSW: {
            RVM_MathOp(&stack, RVM_MUL, 2);
            break;
        }
        case MULW: {
            RVM_MathOp(&stack, RVM_MUL, 4);
            break;
        }
        case MULDW: {
            RVM_MathOp(&stack, RVM_MUL, 8);
            break;
        }
        case DIVB: {
            RVM_MathOp(&stack, RVM_DIV, 1);
            break;
        }
        case DIVSW: {
            RVM_MathOp(&stack, RVM_DIV, 2);
            break;
        }
        case DIVW: {
            RVM_MathOp(&stack, RVM_DIV, 4);
            break;
        }
        case DIVDW: {
            RVM_MathOp(&stack, RVM_DIV, 8);
            break;
        }
        case HALT: {
            break;
        }
        case NOP: {
            break;
        }
        case PUSHB: {
            uint8_t tmp = program[++i];
            spush(&stack, &tmp, 1);
            break;
        }
        case PUSHSW: {
            spush(&stack, &program[++i], 2);
            i++;
            break;
        }
        case PUSHW: {
            spush(&stack, &program[++i], 4);
            i += 3;
            break;
        }
        case PUSHDW: {
            spush(&stack, &program[++i], 8);
            i += 7;
            break;
        }
        case POPB: {
            uint64_t tmp;
            spop(&stack, (uint8_t *)&tmp, 1);
            break;
        }
        case POPSW: {
            uint64_t tmp;
            spop(&stack, (uint8_t *)&tmp, 2);
            break;
        }
        case POPW: {
            uint64_t tmp;
            spop(&stack, (uint8_t *)&tmp, 4);
            break;
        }
        case POPDW: {
            uint64_t tmp;
            spop(&stack, (uint8_t *)&tmp, 8);
            break;
        }
        }
        sDebugPrint(&stack);
    }
    int64_t res;
    spop(&stack, (uint8_t *)&res, 8);
    printf("final: %lld\n", res);
    return 0;
}

int main(int argc, char **argv) {
    uint8_t program[] = {
        NOP, PUSHDW, LONG_TO_BYTE(2ll), PUSHDW, LONG_TO_BYTE(5ll), DIVDW, HALT};
    RVM_RunProgram(program, sizeof(program) / sizeof(program[0]));
    return 0;
}
