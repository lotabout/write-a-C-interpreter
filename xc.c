#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

int token; // current token

enum {LEA, IMM, LC, LI, SC, SI, JMP, CALL, RET, ENT, ADJ, LEV, PUSH, POP, JZ, JNZ, EXIT};

int *text, // text segment
    *data, // data segment
    *stack; // stack

int poolsize; // default size of text/data/stack
int *pc, *bp, *sp, ax, cycle; // virtual machine registers


void next() {
    token = 0;
}

void expression(int level) {

}

void statement() {

}

void eval() {
    int op;
    while (1) {
        op = *pc++; // get next operation code
        if (op == IMM) {ax = *pc++;}                      // load immediate value to ax
        else if (op == LC) {ax = *(char *)ax;}            // load character to ax, address in ax
        else if (op == LI) {ax = *(int *)ax;}             // load integer to ax, address in ax
        else if (op == SC) {ax = *(char *)*sp++ = ax;}    // save character to address, value in ax, address on stack
        else if (op == SI) {*(int *)*sp++ = ax;}          // save integer to address, value in ax, address on stack
        else if (op == PUSH) {*--sp = ax;}                // push the value of ax onto the stack
        else if (op == JMP) {pc = (int *)*pc;}            // jump to the address
        else if (op == JZ) {pc = ax ? pc + 1 : (int *)pc;} // jump if ax is zero
        else if (op == JNZ) {pc = ax ? (int *)pc : pc + 1;} // jump if ax is zero
        else if (op == CALL) {*--sp = (int)(pc+1); pc = (int *)*pc;} // call subroutine
        else if (op == RET) {pc = (int *)*sp++;}          // return from subroutine;
        else if (op == ENT) {*--sp = (int)bp; bp = sp; sp = sp - *pc++;}  // make new stack frame
        else if (op == ADJ) {sp = sp + *pc++;}            // add esp, <size>
        else if (op == LEV) {sp = bp; bp = (int *)*sp++; pc = (int *)*sp++;}  // restore call frame and PC
        else if (op == LEA) {ax = (int)(bp + *pc++);}     // load address for arguments.
    }
}

int main(int argc, char *argv[])
{
    poolsize = 256 * 1024; // arbitrary size

    // allocate memory
    if (!(text = malloc(poolsize))) {
        printf("could not malloc(%d) for text area\n", poolsize);
        return -1;
    }
    if (!(data = malloc(poolsize))) {
        printf("could not malloc(%d) for data area\n", poolsize);
        return -1;
    }
    if (!(stack = malloc(poolsize))) {
        printf("could not malloc(%d) for stack area\n", poolsize);
        return -1;
    }
    memset(text, 0, poolsize);
    memset(data, 0, poolsize);
    memset(stack, 0, poolsize);

    next();

    while (token) {
        statement();
    }

    eval();

    return 0;
}
