# 2. Virtual Machine

In this chapter we're' going to build a virtual machine and design our own
instruction set that runs on the VM. This VM will be the target platform of
the interpreter's code generation phase.

If you've heard of [JVM] and [bytecode], that's what we're trying to build, but
a way way simpler one.


## How Computers Work Internally

There are three components we need to care about: CPU, registers and memory.
Code (assembly instructions) are stored in memory as binary data; the CPU will
retrieve the instruction one by one and execute them; the running states of the
machine are stored in registers.


### Memory

Memory can be used to store data. By data I mean code (assembly instructions)
or other data such as the message you want to print out. All of them are
stored as binary data.

Modern operating systems introduced *Virtual Memory* which maps memory
addresses used by a program (called *virtual address*) into physical addresses
in computer memory.

The benefit of virtual memory is that it can hide the details of a physical
memory from the programs. For example, in a 32-bit machine, all the available
memory address is `2^32 = 4G` while the actual physical memory may be only
`256M`. The program will still think that it can have `4G` memory to use, the
OS will map them to physical ones.

Of course, you don't need to understand the full details of all this. What you
should understand is that a program's usable memory is partitioned into several
segments:

1. `text` segment: for storing code (instructions).
2. `data` segment: for storing initialized data.
   For example `int i = 10;` will need to utilize this segment.
3. `bss` segment: for storing un-initialized data. For example `int i[1000];`
   doesn't need to occupy `1000*4` bytes, because the actual values in the
   array don't matter, thus we can store them in `bss` to save some space.
4. `stack` segment: used for handling the states of function calls, such as
   calling frames and local variables of a function.
5. `heap` segment: use to dynamically allocate memory for program.

Here's an example of the layout of these segments:

```
+------------------+
|    stack   |     |      high address
|    ...     v     |
|                  |
|                  |
|                  |
|                  |
|    ...     ^     |
|    heap    |     |
+------------------+
| bss  segment     |
+------------------+
| data segment     |
+------------------+
| text segment     |      low address
+------------------+
```

Our virtual machine tends to be as simple as possible, thus we don't care about
the `bss` and `heap`. Our interpreter won't support data initialization, thus
we'll merge the `data` and `bss` segments. Moreover, we'll only use the `data`
segment for storing string literals.

We'll drop `heap` as well. This might sounds insane because theoretically the
VM should maintain a `heap` for allocating memory. But hey, an interpreter
itself is also a program which has its heap allocated by our computer. We can
tell the interpreted program to utilize the interpreter's heap by introducing
the `MSET` instruction. I won't say it's cheating, because it reduces the VM's
complexity without subtracting from the knowledge we want to gain on compilers
design.

Thus we'll add the following lines to the code, in the global area:

```c
int *text,            // text segment
    *old_text,        // for dump text segment
    *stack;           // stack
char *data;           // data segment
```

Note the `int` here. What we should write is actually `unsigned` because we'll
store unsigned data (such as pointers/memory addresses) in the `text` segment.
But since we want our interpreter to be bootstrapping (interpret itself), we
don't want to introduce `unsigned`. Furthermore, the `data` is `char *` because
we'll use it to store string literals only.

Finally, add to the `main()` function the code to actually allocate the
segments:

```c
int main() {
    close(fd);
    ...

    // allocate memory for virtual machine
    if (!(text = old_text = malloc(poolsize))) {
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

    ...
    program();
}
```

### Registers

Registers are used to store the running states of computers. There are several
of them in real computers, whereas our VM uses only four:

1. `PC`: **program counter** —
   stores the memory address of the *next* instruction to be run.
2. `SP`: **stack pointer** — always points to the *top* of the stack.
   The stack grows from high address to low address, so whenever we push a new
   element onto the stack, `SP` decreases.
3. `BP`: **base pointer** —
   points to some elements on the stack. It's used in function calls.
4. `AX` —
   a general purpose register used to store the result of an instruction.

In order to fully understand why we need these registers, you need to
understand what states a computer will need to store during computation. They
are just a place to store values. You will get a better understanding after
finishing this chapter.

Well, add some code into the global area:

```c
int *pc, *bp, *sp, ax, cycle; // virtual machine registers
```

And add the initialization code in the `main` function. Note that `pc` should
points to the `main` function of the program being interpreted. But since we
don't have any code generation yet, we'll just skip it for now.

```c
    memset(stack, 0, poolsize);
    ...

    bp = sp = (int *)((int)stack + poolsize);
    ax = 0;

    ...
    program();
```

What's left now is the CPU part. What we should actually do is implementing the
instruction set. We'll save that for a new section.


## Instruction Set

The [instruction set] is a set of instruction that the CPU can understand,
it's the language we need to master in order to talk to CPU. We are going to
design a language for our VM, it's based on x86 instruction set, but much
simpler.

We'll start by adding an `enum` type listing all the instructions that our VM
will understand:

```c
// instructions
enum { LEA ,IMM ,JMP ,CALL,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,
       OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,
       OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT };
```

These instruction are intentionally ordered; as you will find out later,
instructions with arguments come first, while those without arguments comes
after. The only benefit here is for printing debug info. However we will not
rely on this order to introduce them.

### MOV

`MOV` is one of the most fundamental instructions you'll meet. Its job is to
move data into registers or the memory, similar to an assignment expression
in C. There are two arguments in x86's `MOV` instruction:
`MOV dest, source` (Intel style), where `source` can be a number, a register
or a memory address.

But we won't follow x86. On one hand our VM has only one general register
(`AX`), on the other hand it is difficult to determine the type of the
arguments (whether it's a number, register or address). Thus we tear `MOV`
apart into 5 pieces:

1. `IMM <num>` to put immediate `<num>` into register `AX`.
2. `LC` to load a character into `AX` from a memory address which is stored in
   `AX` before execution.
3. `LI` just like `LC` but dealing with integer instead of character.
4. `SC` to store the character in `AX` into the memory address which is stored
   on top of the stack.
5. `SI` just like `SC` but dealing with integer instead of character.

What? I want one `MOV`, not five instructions just to replace it! Don't panic!
You should know that `MOV` is actually a set of instruction that depends on
the `type` of its arguments, so you got `MOVB` for bytes and `MOVW` for words,
etc. Now `LC/SC` and `LI/SI` don't seems that bad, uha?

Well the most important reason is that by turning `MOV` into five sub
instructions, we reduce the complexity a lot! Only `IMM` will accept an
argument now, and now there's no need to worry about its type.

Let's implement it in the `eval` function:

```c
void eval() {
    int op, *tmp;
    while (1) {
        op = *pc++; // get next operation code
        if (op == IMM)       {ax = *pc++;}                                     // load immediate value to ax
        else if (op == LC)   {ax = *(char *)ax;}                               // load character to ax, address in ax
        else if (op == LI)   {ax = *(int *)ax;}                                // load integer to ax, address in ax
        else if (op == SC)   {ax = *(char *)*sp++ = ax;}                       // save character to address, value in ax, address on stack
        else if (op == SI)   {*(int *)*sp++ = ax;}                             // save integer to address, value in ax, address on stack
    }

    ...
    return 0;
}
```

`*sp++` is used to `POP` out one stack element.

You might wonder why we store the address in `AX` register for `LI/LC` while
storing them on top of the stack segment for `SI/SC`. The reason is that the
result of an instruction is stored in `AX` by default. The memory address is
also calculated by an instruction, thus it is more convenient for `LI/LC` to
fetch it directly from `AX`. Also `PUSH` can only push the value of `AX` onto
the stack. So if we want to put an address onto the stack, we'll have to store
it in `AX` anyway, why not skip that?


### PUSH

`PUSH` in x86 can push an immediate value or a register's value onto the
stack. Here in our VM, `PUSH` will push the value in `AX` onto the stack,
only.

```c
else if (op == PUSH) {*--sp = ax;}                                     // push the value of ax onto the stack
```

### JMP

`JMP <addr>` will unconditionally set the value of `PC` register to `<addr>`.

```c
else if (op == JMP)  {pc = (int *)*pc;}                                // jump to the address
```

Notice that `PC` points to the **NEXT** instruction to be executed. Thus `*pc`
stores the argument of `JMP` instruction, i.e. the `<addr>`.


### JZ/JNZ

We'll need conditional jumps to implement `if` statements.
Only two are needed here, to jump when `AX` is `0` or not.

```c
else if (op == JZ)   {pc = ax ? pc + 1 : (int *)*pc;}                   // jump if ax is zero
else if (op == JNZ)  {pc = ax ? (int *)*pc : pc + 1;}                   // jump if ax is not zero
```

### Function Call

It will introduce the calling frame which is hard to understand, so we put it
together to give you an overview. We'll add `CALL`, `ENT`, `ADJ` and `LEV` in
order to support function calls.

A function is a block of code, it may be physically far from the instruction
we are currently executing. So we'll need to `JMP` to the starting point of a
function. Then why introduce a new instruction `CALL`? Because we'll need to
do some bookkeeping: store the current execution position so that the program
can resume after the function call returns.

So we'll need `CALL <addr>` to call the function whose starting point is
`<addr>` and `RET` to fetch the bookkeeping information to resume previous
execution.

```c
else if (op == CALL) {*--sp = (int)(pc+1); pc = (int *)*pc;}           // call subroutine
//else if (op == RET)  {pc = (int *)*sp++;}                              // return from subroutine;
```

We've commented out `RET` because we'll replace it with `LEV` later.

In practice the compiler should deal with more: how to pass the arguments to
a function? How to return the data from the function?

Our convention here for returning a value is to store it into `AX`, no matter
whether you're returning a value or a memory address.
Then what about arguments?

Different languages have [different conventions], here is the standard for C:

1. It is the caller's duty to push the arguments onto stack.
2. After the function call returns, the caller needs to pop out the arguments.
3. The arguments are pushed in reversed order.

Note that we won't follow rule 3. Now let's check how C standard works (from
[Wikipedia](https://en.wikipedia.org/wiki/X86_calling_conventions)):

```c
int callee(int, int, int);

int caller(void)
{
        int i, ret;

        ret = callee(1, 2, 3);
        ret += 5;
        return ret;
}
```

The compiler will generate the following assembly instructions:

```
caller:
        ; make new call frame
        push    ebp
        mov     ebp, esp
        sub     1, esp       ; save stack for variable: i
        ; push call arguments
        push    3
        push    2
        push    1
        ; call subroutine 'callee'
        call    callee
        ; remove arguments from frame
        add     esp, 12
        ; use subroutine result
        add     eax, 5
        ; restore old call frame
        mov     esp, ebp
        pop     ebp
        ; return
        ret
```

The above assembly instructions cannot be achieved in our VM due to several
reasons:

1. `push ebp`, while our `PUSH` doesn't accept arguments at all.
2. `move ebp, esp`, our `MOV` instruction cannot do this.
3. `add esp, 12`, well, still cannot do this (as you'll find out later).

Our instruction set is so simple that we cannot not support function calls! But
we will not surrender to change our design cause it will be too complex for us.
So we add more instructions! Adding new instructions to real computers is an
expensive operation, but not so with virtual machines.


### ENT

`ENT <size>` is called when we are about to enter the function call to "make a
new calling frame". It will store the current `PC` value onto the stack, and
save some space (`<size>` bytes) to store the function's local variables.

```
; make new call frame
push    ebp
mov     ebp, esp
sub     1, esp       ; save stack for variable: i
```

will be translated into:

```c
else if (op == ENT)  {*--sp = (int)bp; bp = sp; sp = sp - *pc++;}      // make new stack frame
```

### ADJ

`ADJ <size>` is to adjust the stack, to "remove arguments from frame." We need
this instruction mainly because our `ADD` doesn't have enough power. So, treat
it as a special add instruction.

```
; remove arguments from frame
add     esp, 12
```

is implemented as:

```c
else if (op == ADJ)  {sp = sp + *pc++;}                                // add esp, <size>
```


### LEV

In case you didn't notice, our instruction set doesn't have `POP`. `POP` in our
compiler would only be used when a function call returns. Which is like this:

```
; restore old call frame
mov     esp, ebp
pop     ebp
; return
ret
```

Thus we'll make another instruction `LEV` to accomplish the work of `MOV`,
`POP` and `RET`:

```c
else if (op == LEV)  {sp = bp; bp = (int *)*sp++; pc = (int *)*sp++;}  // restore call frame and PC
```

### LEA

The instructions introduced above try to solve the problem of
creating/destructing calling frames, one thing left here is how to fetch the
arguments *inside* sub functions.

But we'll check out what a calling frame looks like before learning how to
fetch arguments (note that arguments are pushed in their calling order):

```
sub_function(arg1, arg2, arg3);

|    ....       | high address
+---------------+
| arg: 1        |    new_bp + 4
+---------------+
| arg: 2        |    new_bp + 3
+---------------+
| arg: 3        |    new_bp + 2
+---------------+
|return address |    new_bp + 1
+---------------+
| old BP        | <- new BP
+---------------+
| local var 1   |    new_bp - 1
+---------------+
| local var 2   |    new_bp - 2
+---------------+
|    ....       |  low address
```

So if we need to refer to `arg1`, we need to fetch `new_bp + 4`, which however
cannot be achieved by our poor `ADD` instruction. Thus we will make yet
another special `ADD` to do this: `LEA <offset>`.

```c
else if (op == LEA)  {ax = (int)(bp + *pc++);}                         // load address for arguments.
```

Together with the instructions above, we are able to make function calls.


### Mathematical Instructions

Our VM will provide an instruction for each operators in the C language. Each
operator has two arguments: the first one is stored on the top of the stack,
while the second is stored in `AX`. Their order matters, especially with
operators like `-` and `/`. After the calculation is done, the argument on the
stack will be popped out and the result will be stored in `AX`. So you are not
able to fetch the first argument from the stack after the calculation, please
note that.

```c
else if (op == OR)  ax = *sp++ | ax;
else if (op == XOR) ax = *sp++ ^ ax;
else if (op == AND) ax = *sp++ & ax;
else if (op == EQ)  ax = *sp++ == ax;
else if (op == NE)  ax = *sp++ != ax;
else if (op == LT)  ax = *sp++ < ax;
else if (op == LE)  ax = *sp++ <= ax;
else if (op == GT)  ax = *sp++ >  ax;
else if (op == GE)  ax = *sp++ >= ax;
else if (op == SHL) ax = *sp++ << ax;
else if (op == SHR) ax = *sp++ >> ax;
else if (op == ADD) ax = *sp++ + ax;
else if (op == SUB) ax = *sp++ - ax;
else if (op == MUL) ax = *sp++ * ax;
else if (op == DIV) ax = *sp++ / ax;
else if (op == MOD) ax = *sp++ % ax;
```

### Built-in Instructions

Besides core logic, a program will need input/output mechanisms to be
able to interact with. `printf` in C is one of the commonly used output
functions. `printf` is very complex to implement but unavoidable if our
compiler wants to be bootstrapping (interpret itself) yet it is meaningless for
building a compiler.

Our plan is to create new instructions to build a bridge between the
interpreted program and the interpreter itself. So that we can utilize the
libraries of the host system (your computer that runs the interpreter).

We'll need `exit`, `open`, `close`, `read`, `printf`, `malloc`, `memset` and
`memcmp`:

```c
else if (op == EXIT) { printf("exit(%d)", *sp); return *sp;}
else if (op == OPEN) { ax = open((char *)sp[1], sp[0]); }
else if (op == CLOS) { ax = close(*sp);}
else if (op == READ) { ax = read(sp[2], (char *)sp[1], *sp); }
else if (op == PRTF) { tmp = sp + pc[1]; ax = printf((char *)tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5], tmp[-6]); }
else if (op == MALC) { ax = (int)malloc(*sp);}
else if (op == MSET) { ax = (int)memset((char *)sp[2], sp[1], *sp);}
else if (op == MCMP) { ax = memcmp((char *)sp[2], (char *)sp[1], *sp);}
```

And last, add some error handling:

```c
else {
    printf("unknown instruction:%d\n", op);
    return -1;
}
```


## Test

Now we'll do some "assembly programing" to calculate `10 + 20`:

```c
int main(int argc, char *argv[])
{
    ax = 0;
    ...
    i = 0;
    text[i++] = IMM;
    text[i++] = 10;
    text[i++] = PUSH;
    text[i++] = IMM;
    text[i++] = 20;
    text[i++] = ADD;
    text[i++] = PUSH;
    text[i++] = EXIT;
    pc = text;
    ...
    program();
}
```

Compile the interpreter with `gcc xc-tutor.c` and run it with
`./a.out hello.c`; you should get the following result:

```
exit(30)
```

Note that we specified `hello.c`, but it's not actually being used, we need it
only because the interpreter we've build in the previous chapter needs it.

Well, it seems that our VM works well. :smile:


## Summary

We learned how a computer works internally, and build our own instruction set
modeled after x86 Assembly instructions. We are in fact trying to learn
Assembly language and how it actually works, by building our own version of it.


The code for this chapter can be downloaded from
[GitHub](https://github.com/lotabout/write-a-C-interpreter/tree/step-1),
or cloned by:

```
git clone -b step-1 https://github.com/lotabout/write-a-C-interpreter
```

Note that adding a new instruction would require designing lots of circuits
and cost a lot. But it's almost free to add new instructions in our virtual
machine. We are taking advantage of this by splitting the functions of an
instruction into several, to simplify the implementation.

If you are interested, build your own instruction sets!

<!-----------------------------------------------------------------------------
                               REFERENCE LINKS
------------------------------------------------------------------------------>

[bytecode]: https://en.wikipedia.org/wiki/Bytecode "Wikipedia » Bytecode"
[different conventions]: https://en.wikipedia.org/wiki/Calling_convention "Wikipedia » Calling convention"
[instruction set]: https://en.wikipedia.org/wiki/Instruction_set_architecture "Wikipedia » Instruction set architecture"
[Java virtual machine]: https://en.wikipedia.org/wiki/Java_virtual_machine "Wikipedia » Java virtual machine"
