#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

int token; // current token

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
    next();

    while (token) {
        statement();
    }

    eval();
    
    return 0;
}
