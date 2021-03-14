# 5. Variables

In this chapter we are going to use [EBNF] to describe the grammar of our C
interpreter, and add the support of variables.

The parser is more complicated than the lexer, thus we will split it into three
parts: variables, functions and expressions.


## EBNF grammar

We've talked about [BNF] in the previous chapter, [EBNF] is Extended-BNF.
If you are familiar with regular expression, you should feel right at home.
Personally I think it is more powerful and straightforward than BNF. Here is
the EBNF grammar of our C interpreter, feel free to skip it if you feel it's
too hard to understand.

```
program ::= {global_declaration}+

global_declaration ::= enum_decl | variable_decl | function_decl

enum_decl ::= 'enum' [id] '{' id ['=' 'num'] {',' id ['=' 'num'] '}'

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

We'll leave `expression` to later chapters. Our grammar won't support
function *declaration*, that means recursive calling between functions are not
supported. And since we're bootstrapping, that means our code for its
implementation cannot use any cross-function recursions. (Sorry for the whole
chapter of top-down recursive parsers.)

In this chapter, we'll implement the `enum_decl` and `variable_decl`.


## program()

We've already defined function `program`, turn it into:

```c
void program() {
    // get next token
    next();
    while (token > 0) {
        global_declaration();
    }
}
```

I know that we haven't defined `global_declaration`, sometimes we need wishful
thinking that maybe someone (say Bob) will implement that for you. So you can
focus on the big picture at first, instead of having to drill down into all the
details. That's the essence of top-down thinking.


## global_declaration()

Now it is our duty (not Bob's) to implement `global_declaration`. It will try
to parse variable definitions, type definitions (only `enum` is supported) and
function definitions:

```c
int basetype;    // the type of a declaration, make it global for convenience
int expr_type;   // the type of an expression

void global_declaration() {
    // global_declaration ::= enum_decl | variable_decl | function_decl
    //
    // enum_decl ::= 'enum' [id] '{' id ['=' 'num'] {',' id ['=' 'num'} '}'
    //
    // variable_decl ::= type {'*'} id { ',' {'*'} id } ';'
    //
    // function_decl ::= type {'*'} id '(' parameter_decl ')' '{' body_decl '}'


    int type; // tmp, actual type for variable
    int i; // tmp

    basetype = INT;

    // parse enum, this should be treated alone.
    if (token == Enum) {
        // enum [id] { a = 10, b = 20, ... }
        match(Enum);
        if (token != '{') {
            match(Id); // skip the [id] part
        }
        if (token == '{') {
            // parse the assign part
            match('{');
            enum_declaration();
            match('}');
        }

        match(';');
        return;
    }

    // parse type information
    if (token == Int) {
        match(Int);
    }
    else if (token == Char) {
        match(Char);
        basetype = CHAR;
    }

    // parse the comma seperated variable declaration.
    while (token != ';' && token != '}') {
        type = basetype;
        // parse pointer type, note that there may exist `int ****x;`
        while (token == Mul) {
            match(Mul);
            type = type + PTR;
        }

        if (token != Id) {
            // invalid declaration
            printf("%d: bad global declaration\n", line);
            exit(-1);
        }
        if (current_id[Class]) {
            // identifier exists
            printf("%d: duplicate global declaration\n", line);
            exit(-1);
        }
        match(Id);
        current_id[Type] = type;

        if (token == '(') {
            current_id[Class] = Fun;
            current_id[Value] = (int)(text + 1); // the memory address of function
            function_declaration();
        } else {
            // variable declaration
            current_id[Class] = Glo; // global variable
            current_id[Value] = (int)data; // assign memory address
            data = data + sizeof(int);
        }

        if (token == ',') {
            match(',');
        }
    }
    next();
}
```

Well, that's more than two screens of code! I think that is a direct
translation of the grammar. But to help you understand, I'll explain some of
them:

**Lookahead Token** — The `if (token == xxx)` statement is used to peek at the
next token to decide which production rule to use. For example, if token
`enum` is met, we know it's an enumeration that we're trying to parse. But if a
type if parsed such as `int identifier`, we still cannot tell whether
`identifier` is a variable or function. Thus the parser should continue to
look ahead for the next token, if `(` is met, we are now sure `identifier` is
a function, otherwise it's a variable.

**Variable Type** — Our C interpreter supports pointers, that means pointers
that point to other pointers are also supported, such as `int **data;`. How do
we represents them in code? We've already defined the types that we support:

```c
// types of variable/function
enum { CHAR, INT, PTR };<Paste>
```

So we will use an `int` to store the type. It starts with a base type: `CHAR`
or `INT`. When the type is a pointer that points to a base type such as
`int *data;` we add `PTR` to it: `type = type + PTR;`. The same goes to the
pointer of pointer, we add another `PTR` to the type, etc.


## enum_declaration

The main logic is trying to parse the `,` separated variables. You need to pay
attention to the representation of enumerations.

We will store an enumeration as a global variable. However its `type` is set to
`Num` instead of `Glo` to make it a constant instead of a normal global
variable. The `type` information will later be used in parsing `expression`s.

```c
void enum_declaration() {
    // parse enum [id] { a = 1, b = 3, ...}
    int i;
    i = 0;
    while (token != '}') {
        if (token != Id) {
            printf("%d: bad enum identifier %d\n", line, token);
            exit(-1);
        }
        next();
        if (token == Assign) {
            // like {a=10}
            next();
            if (token != Num) {
                printf("%d: bad enum initializer\n", line);
                exit(-1);
            }
            i = token_val;
            next();
        }

        current_id[Class] = Num;
        current_id[Type] = INT;
        current_id[Value] = i++;

        if (token == ',') {
            next();
        }
    }
}
```

## Misc

Of course `function_declaration` will be introduced in next chapter. `match`
appears a lot. It's a helper function that consume the current token and
fetches the next:

```c
void match(int tk) {
    if (token == tk) {
        next();
    } else {
        printf("%d: expected token: %d\n", line, tk);
        exit(-1);
    }
}
```


## Source Code

You can download the code of this chapter from
[GitHub](https://github.com/lotabout/write-a-C-interpreter/tree/step-3),
or clone with:

```
git clone -b step-3 https://github.com/lotabout/write-a-C-interpreter
```

The code won't run because there are still some unimplemented functions. You
can challenge yourself to fill them out first.


## Summary

EBNF might be difficult to understand because of its syntax (maybe). But it
should be easy to follow this chapter once you can read the syntax. What we do
is to translate EBNF directly into C code. So the parsing is by no means
exciting, but you should pay attention to the representation of each concept.

We'll talk about function definitions in the next chapter, see you then.

<!-----------------------------------------------------------------------------
                               REFERENCE LINKS
------------------------------------------------------------------------------>

[BNF]: https://en.wikipedia.org/wiki/Backus%E2%80%93Naur_Form "Wikipedia: Backus–Naur form"
[EBNF]: https://en.wikipedia.org/wiki/Extended_Backus%E2%80%93Naur_form "Wikipedia: Extended Backus–Naur form"
