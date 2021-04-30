# 1. Skeleton

In this chapter we'll present an overview of the compiler's structure.

Before we start, let me stress again that will be building an **interperter**.
This means we'll be able to run a C source file as if it was a script. The main
reasons behind this choice are twofold:

1. An interpreter differs from a compiler only in the code generation phase,
   thus we'll still learn all the core techniques of building a compiler
   (such as lexical analyzing and parsing).
2. We will build our own virtual machine and [assembly instruction set];
   this will help us understand how computers work.


## The Three Phases of Compiling

Given a source file, the compiler usually carries out three processing phases:

1. **Lexical Analysis**:
   converts source strings into an internal stream of tokens.
2. **Parsing**: consumes the tokens stream and constructs a syntax tree.
3. **Code Generation**:
   walks through the syntax tree and generates code for target platform.

Compiler Construction is so mature that phases one and two can be done by
automation tools. For example, flex can be used for lexical analysis, bison for
parsing. These are powerful tools, which do thousands of things behind the
scene. In order to fully understand how to build a compiler, we're going to
handcraft all three phases, from scratch.

Therefore, we'll build our interpreter in the following steps:

1. Build our own virtual machine and instruction set.
   This will be our target platform in the code generation phase.
2. Build our own lexer for C compilers.
3. Write a [recursive descent parser] on our own.


## The Skeleton of Our Compiler

Modeled after [c4], our compiler includes four main functions:

1. `next()` —
   for lexical analysis; fetches the next token; ignores spaces, tabs, etc.
2. `program()` — parser main entry point.
3. `expression(level)` —
   expressions parser; it will be explained in a later chapter.
4. `eval()` —
   virtual machine entry point; used to interpret target instructions.

Why do we need `expression()` when we already have `program()` for the parser?
That's because the expressions parser is relatively independent and complex,
so we put it into a single module (function).

The code is as follows:

```c
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#define int long long // work with 64-bit target

int token;            // current token
char *src, *old_src;  // pointer to source code string
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

That's quite some code for the first chapter of the tutorial. Nevertheless it's
actually quite simple. The code tries to reads a source file, character by
character, and print them out.

Currently, the lexer function `next()` does nothing except returning the
characters as they are encountered in the source file. The parser's `program()`
doesn't take care of its job either — it doesn't generate any syntax trees, nor
target code.

The important thing here is to understand the meaning of these functions and
how they are hooked together, since they constitute the skeleton of our
interpreter. We'll fill them out step by step, in the upcoming chapters.


## Source Code

The code for this chapter can be downloaded from
[GitHub](https://github.com/lotabout/write-a-C-interpreter/tree/step-0),
or cloned via:

```
git clone -b step-0 https://github.com/lotabout/write-a-C-interpreter
```

> **NOTE** — I might fix bugs later; if you notice any inconsistencies between
the tutorial and the code branches, follow the tutorial. I will only update
code in the master branch.


## Summary

After some boring typing, we now have the simplest compiler: a do-nothing
compiler. In next chapter, we'll implement the `eval` function, i.e. our own
virtual machine. See you then.


<!-----------------------------------------------------------------------------
                               REFERENCE LINKS
------------------------------------------------------------------------------>

[assembly instruction set]: https://en.wikipedia.org/wiki/Instruction_set_architecture "Wikipedia » Instruction set architecture"
[c4]: https://github.com/rswier/c4 "Visit the c4 repository on GitHub"
[recursive descent parser]: https://en.wikipedia.org/wiki/Recursive_descent_parser "Wikipedia » Recursive descent parser"
