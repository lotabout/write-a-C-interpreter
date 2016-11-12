This chapter will be long, please make sure you have enough time for this.
We'll dealing with the last puzzle of our interpreter: expressions.

What is an expression? Well, it is combination of the elements of a
programming language and generates a result, such as function invocation,
variable assignment, calculation using various operators.

We had to pay attention to two things: the precedence of operators and the
target assembly code for operators.

## Precedence of operators

The precedence of operators means we should compute some operators before
others even though the latter may show up first. For example: operator `*`
has higher precedence than operator `+`, so that in expression `2 + 3 * 4`,
the correct calculation result is `2 + (3 * 4)` instead of `(2 + 3) * 4` even
though `+` comes before `+`.

C programming language had already defined the precedence for various
operators, you can refer to [Operator
Precedence](http://en.cppreference.com/w/c/language/operator_precedence).

We'll use stack for handling precedence. One stack for arguments, the other
one for operators. I'll give an example directly: consider `2 + 3 - 4 * 5`,
we'll get the result through the following steps:

1. push `2` onto the stack.
2. operator `+` is met, push it onto the stack, now we are expecting the other
   argument for `+`.
3. `3` is met, push it onto the stack. We are suppose to calculate `2+3`
   immediately, but we are not sure whether `3` belongs to the operator with
   higher precedence, so leave it there.
4. operator `-` is met. `-` has the same precedence as `+`, so we are sure
   that the value `3` on the stack belongs to `+`. Thus the pending
   calculation `2+3` is evaluated. `3`, `+`, `2` are poped from the stack and
   the result `5` is pushed back. Don't forget to push `-` onto the stack.
5. `4` is met, but we are not sure if it 'belongs' to `-`, leave it there.
6. `*` is met and it has higher precedence than `-`, now we have two operators
   pending.
7. `5` is met, and still not sure whom `5` belongs to. Leave it there.
8. The expression end. Now we are sure that `5` belongs to the operator lower
   on the stack: `*`, pop them out and push the result `4 * 5 = 20` back.
9. Continue to pop out items push the result `5 - 20 = -15` back.
10. Now the operator stack is empty, pop out the result: `-15`

```
// after step 1, 2
|      |
+------+
| 3    |   |      |
+------+   +------+
| 2    |   | +    |
+------+   +------+

// after step 4
|      |   |      |
+------+   +------+
| 5    |   | -    |
+------+   +------+

// after step 7
|      |
+------+
| 5    |
+------+   +------+
| 4    |   | *    |
+------+   +------+
| 5    |   | -    |
+------+   +------+
```

As described above, we had to make sure that the right side of argument
belongs to current operator 'x', thus we have to look right of the expression,
find out and calculate the ones that have higher precedence. Then do the
calculation for current operator 'x'.

Finally, we need to consider precedence for only binary/ternary operators.
Because precedence means different operators try to "snatch" arguments from
each other, while unary operators are the strongest.

## Unary operators

Unary operators are strongest, so we serve them first. Of course, we'll also
parse the arguments(i.e. variables, number, string, etc) for operators.

We've already learned the parsing

### Constant

First comes numbers, we use `IMM` to load it into `AX`:

```c
if (token == Num) {
    match(Num);
    // emit code

    *++text = IMM;
    *++text = token_val;
    expr_type = INT;
}
```

Next comes string literals, however C support this kind of string
concatination:

```c
char *p;
p = "first line"
    "second line";

// which equals to:

char *p;
p = "first linesecond line";
```

```c
else if (token == '"') {
    // emit code
    *++text = IMM;
    *++text = token_val;
    match('"');

    // store the rest strings
    while (token == '"') {
        match('"');
    }

    // append the end of string character '\0', all the data are default
    // to 0, so just move data one position forward.
    data = (char *)(((int)data + sizeof(int)) & (-sizeof(int)));
    expr_type = PTR;
}
```

### sizeof

It is an unary operator, we'll have to know to type of its argument which we
are familiar with.

```c
else if (token == Sizeof) {
    // sizeof is actually an unary operator
    // now only `sizeof(int)`, `sizeof(char)` and `sizeof(*...)` are
    // supported.
    match(Sizeof);
    match('(');
    expr_type = INT;

    if (token == Int) {
        match(Int);
    } else if (token == Char) {
        match(Char);
        expr_type = CHAR;
    }

    while (token == Mul) {
        match(Mul);
        expr_type = expr_type + PTR;
    }

    match(')');

    // emit code
    *++text = IMM;
    *++text = (expr_type == CHAR) ? sizeof(char) : sizeof(int);
    expr_type = INT;
}
```

Note that only `sizeof(int)`, `sizeof(char)` and `sizeof(pointer type ...)`
are supported, and the type of the result is `int`.

### Variable and function invocation

They all starts with an `Id` token, thus are handled together.

```c
else if (token == Id) {
    // there are several type when occurs to Id
    // but this is unit, so it can only be
    // 1. function call
    // 2. Enum variable
    // 3. global/local variable
    match(Id);

    id = current_id;

    if (token == '(') {
        // function call
        match('(');

        // ①
        // pass in arguments
        tmp = 0; // number of arguments
        while (token != ')') {
            expression(Assign);
            *++text = PUSH;
            tmp ++;

            if (token == ',') {
                match(',');
            }
        }
        match(')');

        // ②
        // emit code
        if (id[Class] == Sys) {
            // system functions
            *++text = id[Value];
        }
        else if (id[Class] == Fun) {
            // function call
            *++text = CALL;
            *++text = id[Value];
        }
        else {
            printf("%d: bad function call\n", line);
            exit(-1);
        }

        // ③
        // clean the stack for arguments
        if (tmp > 0) {
            *++text = ADJ;
            *++text = tmp;
        }
        expr_type = id[Type];
    }
    else if (id[Class] == Num) {
        // ④
        // enum variable
        *++text = IMM;
        *++text = id[Value];
        expr_type = INT;
    }
    else {
        // ⑤
        // variable
        if (id[Class] == Loc) {
            *++text = LEA;
            *++text = index_of_bp - id[Value];
        }
        else if (id[Class] == Glo) {
            *++text = IMM;
            *++text = id[Value];
        }
        else {
            printf("%d: undefined variable\n", line);
            exit(-1);
        }

        //⑥
        // emit code, default behaviour is to load the value of the
        // address which is stored in `ax`
        expr_type = id[Type];
        *++text = (expr_type == Char) ? LC : LI;
    }
}
```

①: Notice we are using the normal order to push the arguments which
corresponds to the implementation of our virtual machine. However C standard
push the argument in reverse order.

②: Note how we support `printf`, `read`, `malloc` and other built in
functions in our virtual machine. These function calls have specific assembly
instructions while normal functions are compiled into `CALL <addr>`.

③: Remove the arguments on the stack, we modifies the stack pointer directly
because we don't care about the values.

④: Enum variables are treated as constant numbers.

⑤: Load the values of variable, use the `bp + offset` style for local
variable(refer to chapter 7), use `IMM` to load the address of global
variables.

⑥: Finally load the value of variables using `LI/LC` according to their type.

You might ask how to deal with expressions like `a[10]` if we use `LI/LC` to
load immediatly when an identifier is met? We might modify existing assembly
instructions according to the operator after the identifier, as you'll see
later.

### Casting

Perhaps you've notice that we use `expr_type` to store the type of the return
value of an expression. Type Casting is for changing the type of the return
value of an expression.

```c
else if (token == '(') {
    // cast or parenthesis
    match('(');
    if (token == Int || token == Char) {
        tmp = (token == Char) ? CHAR : INT; // cast type
        match(token);
        while (token == Mul) {
            match(Mul);
            tmp = tmp + PTR;
        }

        match(')');

        expression(Inc); // cast has precedence as Inc(++)

        expr_type  = tmp;
    } else {
        // normal parenthesis
        expression(Assign);
        match(')');
    }
}
```

### Dereference/Indirection

`*a` in C is to get the object pointed by pointer `a`. It is essential to find
out the type of pointer `a`. Luckily the type information will be stored in
variable `expr_type` when an expression ends.

```c
else if (token == Mul) {
    // dereference *<addr>
    match(Mul);
    expression(Inc); // dereference has the same precedence as Inc(++)

    if (expr_type >= PTR) {
        expr_type = expr_type - PTR;
    } else {
        printf("%d: bad dereference\n", line);
        exit(-1);
    }

    *++text = (expr_type == CHAR) ? LC : LI;
}
```

### Address-Of

In section "Variable and function invocation", we said we will modify `LI/LC`
instructions dynamically, now it is the time. We said that we'll load the
address of a variable first and call `LI/LC` instruction to load the actual
content according to the type:

```
IMM <addr>
LI
```

Then for getting `address-of` a variable, ignoring `LI/LC` instruction will do
the trick.

```c
else if (token == And) {
    // get the address of
    match(And);
    expression(Inc); // get the address of
    if (*text == LC || *text == LI) {
        text --;
    } else {
        printf("%d: bad address of\n", line);
        exit(-1);
    }

    expr_type = expr_type + PTR;
}
```

### Logical NOT

We don't have logical not instruction in our virtual machine, thus we'll
compare the result to `0` which represents `False`:

```c
else if (token == '!') {
    // not
    match('!');
    expression(Inc);

    // emit code, use <expr> == 0
    *++text = PUSH;
    *++text = IMM;
    *++text = 0;
    *++text = EQ;

    expr_type = INT;
}
```

### Bitwise NOT

We don't have corresponding instruction in our virtual machine either. Thus we
use `XOR` to implement, e.g. `~a = a ^ 0xFFFF`.

```c
else if (token == '~') {
    // bitwise not
    match('~');
    expression(Inc);

    // emit code, use <expr> XOR -1
    *++text = PUSH;
    *++text = IMM;
    *++text = -1;
    *++text = XOR;

    expr_type = INT;
}
```

### Unary plus and Unary minus

Use `0 - x` to implement `-x`:

```c
else if (token == Add) {
    // +var, do nothing
    match(Add);
    expression(Inc);
    expr_type = INT;

} else if (token == Sub) {
    // -var
    match(Sub);

    if (token == Num) {
        *++text = IMM;
        *++text = -token_val;
        match(Num);
    } else {
        *++text = IMM;
        *++text = -1;
        *++text = PUSH;
        expression(Inc);
        *++text = MUL;
    }

    expr_type = INT;
}
```

### Increment and Decrement

The precedence for increment or decrement is related to the position of the
operator. Such that `++p` has higher precedence than `p++`. Now we are dealing
with `++p`.

```c
else if (token == Inc || token == Dec) {
    tmp = token;
    match(token);
    expression(Inc);
    // ①
    if (*text == LC) {
        *text = PUSH;  // to duplicate the address
        *++text = LC;
    } else if (*text == LI) {
        *text = PUSH;
        *++text = LI;
    } else {
        printf("%d: bad lvalue of pre-increment\n", line);
        exit(-1);
    }
    *++text = PUSH;
    *++text = IMM;

    // ②
    *++text = (expr_type > PTR) ? sizeof(int) : sizeof(char);
    *++text = (tmp == Inc) ? ADD : SUB;
    *++text = (expr_type == CHAR) ? SC : SI;
}
```

For `++p` we need to access `p` twice: one for load the value, one for storing
the incremented value, that's why we need to `PUSH` (①) it once.

② deal with cases when `p` is pointer.

## Binary Operators

Now we need to deal with the precedence of operators. We will scan to the
right of current operator, until one that has **the same or lower** precedence
than the current operator is met.

Let's recall the tokens that we've defined, they are order by their
precedences from low to high. That mean `Assign` is the lowest and `Brak`(`[`)
is the highest.

```c
enum {
  Num = 128, Fun, Sys, Glo, Loc, Id,
  Char, Else, Enum, If, Int, Return, Sizeof, While,
  Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak
};
```

Thus the argument `level` in calling `expression(level)` is actually used to
indicate the precedence of current operator, thus the skeleton for parsing
binary operators is:

```c
while (token >= level) {
    // parse token for binary operator and postfix operator
}
```

Now we know how to deal with precedence, let's check how operators are
compiled into assembly instructions.

### Assignment

`Assign` has the lowest precedence. Consider expression `a = (expression)`,
we've already generated instructions for `a` as:

```
IMM <addr>
LC/LI
```

After the expression on the right side of `=` is evaluated, the result will be
stored in register `AX`. In order to assign the value in `AX` to the variable
`a`, we need to change the original instructions into:

```
IMM <addr>
PUSH
SC/SI
```

Now we can understand how to parse `Assign`:

```c
tmp = expr_type;
if (token == Assign) {
    // var = expr;
    match(Assign);
    if (*text == LC || *text == LI) {
        *text = PUSH; // save the lvalue's pointer
    } else {
        printf("%d: bad lvalue in assignment\n", line);
        exit(-1);
    }
    expression(Assign);

    expr_type = tmp;
    *++text = (expr_type == CHAR) ? SC : SI;
}
```

### Ternary Conditional

That is `? :` in C. It is a operator version for `if` statement. The target
instructions are almost identical to `if`:

```c
else if (token == Cond) {
    // expr ? a : b;
    match(Cond);
    *++text = JZ;
    addr = ++text;
    expression(Assign);
    if (token == ':') {
        match(':');
    } else {
        printf("%d: missing colon in conditional\n", line);
        exit(-1);
    }
    *addr = (int)(text + 3);
    *++text = JMP;
    addr = ++text;
    expression(Cond);
    *addr = (int)(text + 1);
}
```

### Logical Operators

Two of them: `||` and `&&`. Their corresponding assembly instructions are:

```c
<expr1> || <expr2>     <expr1> && <expr2>

  ...<expr1>...          ...<expr1>...
  JNZ b                  JZ b
  ...<expr2>...          ...<expr2>...
b:                     b:
```

Source code as following:

```c
else if (token == Lor) {
    // logic or
    match(Lor);
    *++text = JNZ;
    addr = ++text;
    expression(Lan);
    *addr = (int)(text + 1);
    expr_type = INT;
}
else if (token == Lan) {
    // logic and
    match(Lan);
    *++text = JZ;
    addr = ++text;
    expression(Or);
    *addr = (int)(text + 1);
    expr_type = INT;
}
```

### Mathematical Operators

Including `|`, `^`, `&`, `==`, `!=`, `<=`, `>=`, `<`, `>`, `<<`, `>>`, `+`,
`-`, `*`, `/`, `%`. They are similar to implement, we'll give `^` as an
example:

```
<expr1> ^ <expr2>

...<expr1>...          <- now the result is on ax
PUSH
...<expr2>...          <- now the value of <expr2> is on ax
XOR
```

Thus the source code is:

```c
else if (token == Xor) {
    // bitwise xor
    match(Xor);
    *++text = PUSH;
    expression(And);
    *++text = XOR;
    expr_type = INT;
}
```

Quite easy, hah? There are still something to mention about addition and
substraction for pointers. A pointer plus/minus some number equals to the
shiftment for a pointer according to its type. For example, `a + 1` will shift
for 1 byte if `a` is `char *`, while 4 bytes(in 32 bit machine) if a is `int
*`.

Also, substraction for two pointers will give the number of element between
them, thus need special treatment.

Take addition as example:

```
<expr1> + <expr2>

normal         pointer

<expr1>        <expr1>
PUSH           PUSH
<expr2>        <expr2>     |
ADD            PUSH        | <expr2> * <unit>
               IMM <unit>  |
               MUL         |
               ADD
```

It means if `<expr1>` is a pointer, we need to multiply `<expr2>` with the
number of bytes that `<expr1>`'s type occupies.

```c
else if (token == Add) {
    // add
    match(Add);
    *++text = PUSH;
    expression(Mul);
    expr_type = tmp;

    if (expr_type > PTR) {
        // pointer type, and not `char *`
        *++text = PUSH;
        *++text = IMM;
        *++text = sizeof(int);
        *++text = MUL;
    }
    *++text = ADD;
}
```

You can try to implement substraction by your own or refer to the repository.

### Increment and Decrement Again

Now we deal with the postfix version, e.g. `p++` or `p--`. Different from the
prefix version, the postfix version need to store the value **before**
increment/decrement on `AX` after the increment/decrement. Let's compare them:

```c
// prefix version
*++text = PUSH;
*++text = IMM;
*++text = (expr_type > PTR) ? sizeof(int) : sizeof(char);
*++text = (tmp == Inc) ? ADD : SUB;
*++text = (expr_type == CHAR) ? SC : SI;

// postfix version
*++text = PUSH;
*++text = IMM;
*++text = (expr_type > PTR) ? sizeof(int) : sizeof(char);
*++text = (token == Inc) ? ADD : SUB;
*++text = (expr_type == CHAR) ? SC : SI;
*++text = PUSH;                                             //
*++text = IMM;                                              // Inversion of increment/decrement
*++text = (expr_type > PTR) ? sizeof(int) : sizeof(char);   //
*++text = (token == Inc) ? SUB : ADD;                       //
```

### Indexing

You may already know that `a[10]` equals to `*(a + 10)` in C language. The
interpreter is trying to do the same:

```c
else if (token == Brak) {
    // array access var[xx]
    match(Brak);
    *++text = PUSH;
    expression(Assign);
    match(']');

    if (tmp > PTR) {
        // pointer, `not char *`
        *++text = PUSH;
        *++text = IMM;
        *++text = sizeof(int);
        *++text = MUL;
    }
    else if (tmp < PTR) {
        printf("%d: pointer type expected\n", line);
        exit(-1);
    }
    expr_type = tmp - PTR;
    *++text = ADD;
    *++text = (expr_type == CHAR) ? LC : LI;
}
```

## Code

We need to initialize the stack for our virtual machine besides all the
expressions in the above sections so that `main` function is correctly called,
and exit when `main` exit.

```c
int *tmp;
// setup stack
sp = (int *)((int)stack + poolsize);
*--sp = EXIT; // call exit if main returns
*--sp = PUSH; tmp = sp;
*--sp = argc;
*--sp = (int)argv;
*--sp = (int)tmp;
```

Last, due to the limitation of our interpreter, all the definitions of variables
should be put before all expressions, just like the old C compiler requires.

You can download all the code on
[Github](https://github.com/lotabout/write-a-C-interpreter/tree/step-6) or
clone it by:

```
git clone -b step-6 https://github.com/lotabout/write-a-C-interpreter
```

Compile it by `gcc -o xc-tutor xc-tutor.c` and run it by `./xc-tutor hello.c`
to check the result.

Our code is bootstraping, that means our interpreter can parse itself, so that
you can run our C interpreter inside itself by `./xc-tutor xc-tutor.c
hello.c`.

You might need to compile with `gcc -m32 -o xc-tutor xc-tutor.c` if you use a
64 bit machine.

## Summary

This chapter is long mainly because there are quite a lot of operators there.
You might pay attention to:

1. How to call `expression` recursively to handle the precedence of operators.
2. What is the target assembly instructions for each operator.

Congratulations! You've build a runnable C interpeter all by yourself!
