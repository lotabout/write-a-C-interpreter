We've already seen how variable definitions are parsed in our interpreter. Now
it's time for function definitions. (note it is definition, not declaration,
thus our interpreter don't support recursion across functions.)

## EBNF Grammar

Let's start be refreshing our memories about EBNF grammar introduced in last
chapter, we've already implement `program`, `global_declaration` and
`enum_decl`. We'll deal with part of `variable_decl`, `function_decl`,
`parameter_decl` and `body_decl`. The rest will be covered in next chapter.

```
variable_decl ::= type {'*'} id { ',' {'*'} id } ';'

function_decl ::= type {'*'} id '(' parameter_decl ')' '{' body_decl '}'

parameter_decl ::= type {'*'} id {',' type {'*'} id}

body_decl ::= {variable_decl}, {statement}

statement ::= non_empty_statement | empty_statement

non_empty_statement ::= if_statement | while_statement | '{' statement '}'
                     | 'return' expression | expression ';'

if_statement ::= 'if' '(' expression ')' statement ['else' non_empty_statement]

while_statement ::= 'while' '(' expression ')' non_empty_statement
```

## Function definition

Recall that we've already encountered functions when handling
`global_declaration`:

```c
...
if (token == '(') {
    current_id[Class] = Fun;
    current_id[Value] = (int)(text + 1); // the memory address of function
    function_declaration();
} else {
...
```

The type for current identifier(i.e. function name) had already been set
correctly. The above chunk of code set the type(i.e. `Fun`) and the
address in `text segment` for the function. Here comes `parameter_decl` and
`body_decl`.

## Parameters and Assembly Output

Before we get our hand dirty, we have to understand the assembly code that
will be output for a function. Consider the following:

```c
int demo(int param_a, int *param_b) {
    int local_1;
    char local_2;

    ...
}
```

When `demo` is called, its calling frame(states of stack) will look like the
following(please refer the the VM of chapter 2):

```
|    ....       | high address
+---------------+
| arg: param_a  |    new_bp + 3
+---------------+
| arg: param_b  |    new_bp + 2
+---------------+
|return address |    new_bp + 1
+---------------+
| old BP        | <- new BP
+---------------+
| local_1       |    new_bp - 1
+---------------+
| local_2       |    new_bp - 2
+---------------+
|    ....       |  low address
```

The key point here is no matter it is parameter(e.g. `param_a`) or local
variable (e.g. `local_1`), they are all store on **stack**. Thus they are
referred to by pointer `new_bp` and relative shfitment, while global variables
which are stored in `text segment` are refered to by direct address. So we
need to know the number of parameters and the shiftment of each.

## Skeleton for Parsing Function

```c
void function_declaration() {
    // type func_name (...) {...}
    //               | this part

    match('(');
    function_parameter();
    match(')');
    match('{');
    function_body();
    //match('}');                 //  ①

    // ②
    // unwind local variable declarations for all local variables.
    current_id = symbols;
    while (current_id[Token]) {
        if (current_id[Class] == Loc) {
            current_id[Class] = current_id[BClass];
            current_id[Type]  = current_id[BType];
            current_id[Value] = current_id[BValue];
        }
        current_id = current_id + IdSize;
    }
}
```

Note that we are supposed to consume the last `}` character in ①. But we don't
because `variable_decl` and `function_decl` are parsed together(because of the
same prefix in EBNF grammar) inside `global_declaration`. `variable_decl` ends
with `;` while `function_decl` ends with `}`. If `}` is consumed, the `while`
loop in `global_declaration` won't be able to know that a `function_decl`
parsing is end. Thus we leave it to `global_declaration` to consume it.

What ② is trying to do is unwind local variable declarations for all local
variables. As we know, local variables can have the same name as global ones,
once it happans, global ones will be shadowed. So we should recover the status
once we exit the function body. Informations about global variables are backed
up to fields `BXXX`, so we iterate over all identifiers to recover.

## function_parameter()

```
parameter_decl ::= type {'*'} id {',' type {'*'} id}
```

It is quite straightforward except we need to remember the types and position
of the parameter.

```c
int index_of_bp; // index of bp pointer on stack

void function_parameter() {
    int type;
    int params;
    params = 0;
    while (token != ')') {
        // ①

        // int name, ...
        type = INT;
        if (token == Int) {
            match(Int);
        } else if (token == Char) {
            type = CHAR;
            match(Char);
        }

        // pointer type
        while (token == Mul) {
            match(Mul);
            type = type + PTR;
        }

        // parameter name
        if (token != Id) {
            printf("%d: bad parameter declaration\n", line);
            exit(-1);
        }
        if (current_id[Class] == Loc) {
            printf("%d: duplicate parameter declaration\n", line);
            exit(-1);
        }

        match(Id);

        //②
        // store the local variable
        current_id[BClass] = current_id[Class]; current_id[Class]  = Loc;
        current_id[BType]  = current_id[Type];  current_id[Type]   = type;
        current_id[BValue] = current_id[Value]; current_id[Value]  = params++;   // index of current parameter

        if (token == ',') {
            match(',');
        }
    }

    // ③
    index_of_bp = params+1;
}
```

Part ① is the same to what we've seen in `global_declaration` which is used to
parse the type for the parameter.

Part ② is to backup the information for global variables which will be
shadowed by local variables. The position of current parameter is stored in
field `Value`.

Part ③ is used to calculate the position of pointer `bp` which corresponds to
`new_bp` that we talked about in the above section.

## function_body()

Different with modern C, our interpreter requires that all the definitinos of
variables that are used in current function should be put at the beginning of
current function. This rule is actually the same to ancient C compilers.

```c
void function_body() {
    // type func_name (...) {...}
    //                   -->|   |<--

    // ... {
    // 1. local declarations
    // 2. statements
    // }

    int pos_local; // position of local variables on the stack.
    int type;
    pos_local = index_of_bp;

    // ①
    while (token == Int || token == Char) {
        // local variable declaration, just like global ones.
        basetype = (token == Int) ? INT : CHAR;
        match(token);

        while (token != ';') {
            type = basetype;
            while (token == Mul) {
                match(Mul);
                type = type + PTR;
            }

            if (token != Id) {
                // invalid declaration
                printf("%d: bad local declaration\n", line);
                exit(-1);
            }
            if (current_id[Class] == Loc) {
                // identifier exists
                printf("%d: duplicate local declaration\n", line);
                exit(-1);
            }
            match(Id);

            // store the local variable
            current_id[BClass] = current_id[Class]; current_id[Class]  = Loc;
            current_id[BType]  = current_id[Type];  current_id[Type]   = type;
            current_id[BValue] = current_id[Value]; current_id[Value]  = ++pos_local;   // index of current parameter

            if (token == ',') {
                match(',');
            }
        }
        match(';');
    }

    // ②
    // save the stack size for local variables
    *++text = ENT;
    *++text = pos_local - index_of_bp;

    // statements
    while (token != '}') {
        statement();
    }

    // emit code for leaving the sub function
    *++text = LEV;
}
```

You should be familiar with ①, it had been repeated several times.

Part ② is writing assembly code into text segment. In the VM chapter, we said
we have to preserve spaces for local variables on stack, well, this is it.

## Code

You can download the code of this chapter from [Github](https://github.com/lotabout/write-a-C-interpreter/tree/step-4), or clone with:

```
git clone -b step-4 https://github.com/lotabout/write-a-C-interpreter
```

The code still won't run because there are still some un-implemented
functions. You can challange yourself to fill them out first.

## Summary

The code of this chapter isn't long, most of the part are used to parse
variables and much of them are duplicated. The parsing for parameter and local
variables are almost the same, but the stored information are different.

Of course, you may want to review the VM chapter(chapter 2) to get better
understanding of the expected output for function, so as to understand why
would we want to gather such information. This is what we called
"domain knowledge".

We'll deal with `if`, `while` next chapter, see you then.
