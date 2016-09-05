In this chapter we will have an overview of the compiler's structure.

Before we start, I'd like to restress that it is **interperter** that we want
to build. That means we can run a C source file just lik a script. It is
chosen mainly for two reasons:

1. Interpreter differs from Compiler only in code generation phase, thus we'll
   still learn all the core techniques of building a compiler(such as lexical
   analyzing and parsing).
2. We will build our own virtual machine and assembly instructions, that would
   help us to understand how computers work.

## Three Phases

Given a source file, normally the compiler will cast three phases of
processing:

1. Lexical Analysis: converts source strings into internal token stream.
2. Parsing: consumes token stream and constructs syntax tree.
3. Code Generation: walk through the syntax tree and generate code for target
   platform.

Compiler Construction had been so mature that part 1 & 2 can be done by
automation tools. For example, flex can be used for lexical analysis, bison
for parsing. They are powerful but do thousands of things behind the scene. In
order to fully understand how to build a compiler, we are going to build them
all from scratch.

Thus we will build our interpreter in the following steps:

1. Build our own virtual machine and instruction set. This is the target
   platform that will be using in our code generation phase.
2. Build our own lexer for C compiler.
3. Write a recusion descent parser on our own.

## Skeleton of our compiler


Modeling after c4, our compiler includes 4 main functions:

1. `next()` for lexical analysis; get the next token; will ignore spaces tabs
   etc.
2. `program()` main entrance for parser.
3. `expression(level)`: parse expression; level will be explained in later
   chapter.
4. `eval()`: the entrance for virtual machine; used to interpret target
   instructions.

Why would `expression` exist when we have `program` for parser? That's because
the parser for expressions is relatively independent and complex, so we put it
into a single module(function).

The code is as following:

```c
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

int token;            // current token
char *src, *old_src;  // pointer to source code string;
int poolsize;         // default size of text/data/stack
int line;             // line number

void next() {
    token = *src++;
    return;
}

void expression(int level) {
    // do nothing
}

void program() {
    next();                  // get next token
    while (token > 0) {
        printf("token is: %c\n", token);
        next();
    }
}

int eval() { // do nothing yet
    return 0;
}

int main(int argc, char **argv)
{
    int i, fd;

    argc--;
    argv++;

    poolsize = 256 * 1024; // arbitrary size
    line = 1;

    if ((fd = open(*argv, 0)) < 0) {
        printf("could not open(%s)\n", *argv);
        return -1;
    }

    if (!(src = old_src = malloc(poolsize))) {
        printf("could not malloc(%d) for source area\n", poolsize);
        return -1;
    }

    // read the source file
    if ((i = read(fd, src, poolsize-1)) <= 0) {
        printf("read() returned %d\n", i);
        return -1;
    }

    src[i] = 0; // add EOF character
    close(fd);

    program();
    return eval();
}
```

That's quite some code for the first chapter of the article. Nevertheless it
is actually simple enough. The code tries to reads in a source file, character
by character and print them out.

Currently the lexer `next()` does nothing but returning the characters as they
are in the source file. The parser `program()` doesn't take care of its job
either, no syntax trees are generated, no target codes are generated.

The important thing here is to understand the meaning of these functions and
how they are hooked together as they are the skeleton of our interpreter.
We'll fill them out step by step in later chapters.

## Code

The code for this chapter can be downloaded from
[Github](https://github.com/lotabout/write-a-C-interpreter/tree/step-0), or
clone by:

```
git clone -b step-0 https://github.com/lotabout/write-a-C-interpreter
```

Note that I might fix bugs later, and if there is any incosistance between the
artical and the code branches, follow the article. I would only update code in
the master branch.

## Summary

After some boring typing, we have the simplest compiler: a do-nothing
compiler. In next chapter, we will implement the `eval` function, i.e. our own
virtual machine. See you then.
