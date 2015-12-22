#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

int token; // current token

enum {LEA, MOV, JMP, CALL, PUSH, POP, JZ, JNZ, EXIT};

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

    
    int tmp;
    pc = text;
    sp = bp = (int *)((int)stack + poolsize);
    // setup stack for exit of main function
    *--sp = EXIT;
    *--sp = PUSH; 
    tmp = sp;
    *--sp = argc;
    *--sp = (int)argv;
    *--sp = (int)tmp;

    next();

    while (token) {
        statement();
    }

    eval();
    
    return 0;
}
