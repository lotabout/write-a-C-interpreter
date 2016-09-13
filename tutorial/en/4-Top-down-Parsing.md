In this chapter we will build a simple calculator using the top-down parsing
technique. This is the preparation before we start to implement the parser.

I will introduce a small set of theories but not gurantee to be absolute
correct, please consult your textbook if you have any confusion.

## Top-down parsing

Traditionally, we have top-down parsing and bottom-up parsing. Top-down method
will start with a non-terminator and recursively check the source code to
replace the non-terminators with its alternatives until no non-terminator is
left.

You see I used the top-down method for explaining "top-down" because you'll
have to know what a "non-terminator" is to understand the above graph. But I
havn't tell you what that is. We will tell in the next section. For now,
consider "top-down" is try to tear down a big object into small pieces.

On the other hand "bottom-up" parsing is trying to combine small objects into
a big one. It is often used in automation tools that generate parsers.

## Terminator and Non-terminator

They are terms used in
[BNF](https://en.wikipedia.org/wiki/Backus%E2%80%93Naur_Form) (Backus–Naur
Form) which is a language used to describe grammars. A simple elementary
arithmetic calulater in BNF will be:

```
<expr> ::= <expr> + <term>
         | <expr> - <term>
         | <term>

<term> ::= <term> * <factor>
         | <term> / <factor>
         | <factor>

<factor> ::= ( <expr> )
           | Num
```

The one enbraced by `<>` is called a `Non-terminator`. They got the name
because we can replace them with the ones on the right hand of `::=`.
`|` means alternative that means you can replace `<term>` with any one of
`<term> * <factor>`, `<term> / <factor>` or `<factor>`. Those do not appear on
the left side of `::=` is called `Terminator` such as `+`, `(`, `Num`, etc.
They often corresponds to the tokens we got from lexer.

## Top-down Example for Simple Calculator

Parse tree is the inner structure we got after the parser consumes all the
tokens and finish all the parsing. Let's take `3 * (4 + 2)` as an example to
show the connections between BNF grammer, parse tree and top-down parsing.

Top-down parsing starts from a starting non-terminator which is `<term>` in
our example. You can specify it in practice, but also defaults to the first
non-terminator we encountered.

```
1. <expr> => <expr>
2.           => <term>        * <factor>
3.              => <factor>     |
4.                 => Num (3)   |
5.                              => ( <expr> )
6.                                   => <expr>           + <term>
7.                                      => <term>          |
8.                                         => <factor>     |
9.                                            => Num (4)   |
10.                                                        => <factor>
11.                                                           => Num (2)
```

You can see that each step we replace a non-terminator using one of its
alternatives(top-down) Until all of the sub-items are replaced to
terminators(bottom). Some non-terminators are used recursively such as
`<expr>`.

## Advantages of Top-down Parsing

As you can see in the above example, the parsing step is similar to the BNF
grammar. Which means it is easy to convert the grammar into actual code by
converting a production rule (`<...> ::= ...`) into a function with the same
name.

One question arises here: how do you know which alternative to apply? Why do
you choose `<expr> ::= <term> * <factor>` over `<expr> ::= <term> / <factor>`?
That's right, we `lookahead`! We peek the next token and it is `*` so it is the
first one to apply.

However, top-down parsing requires the grammar should not have left-recursion.

## Left-recursion

Suppose we have a grammer like this:

```
<expr> ::= <expr> + Num
```

we can translate it into function:

```
int expr() {
    expr();
    num();
}
```

As you can see, function `expr` will never exit! In the grammar,
non-terminator `<expr>` is used recursively and appears immediately after
`::=` which causes left-recursion.

Lucily, most left-recursive grammers(maybe all? I don't remember) can be
properly transformed into non left-recursive equivalent ones. Our grammar for
calculator can be converted into:

```
<expr> ::= <term> <expr_tail>

<expr_tail> ::= + <term> <expr_tail>
              | - <term> <expr_tail>
              | <empty>
              
<term> ::= <factor> <term_tail>

<term_tail> ::= * <factor> <term_tail>
              | / <factor> <term_tail>
              | <empty>
              
<factor> ::= ( <expr> )
           | Num
```

You should check out your textbook for more information.

## Implementation

The following code is directly converted from the grammar. Notice how
straightforward it is:

```c
int expr();

int factor() {
    int value = 0;
    if (token == '(') {
        match('(');
        value = expr();
        match(')');
    } else {
        value = token_val;
        match(Num);
    }
    return value;
}

int term_tail(int lvalue) {
    if (token == '*') {
        match('*');
        int value = lvalue * factor();
        return term_tail(value);
    } else if (token == '/') {
        match('/');
        int value = lvalue / factor();
        return term_tail(value);
    } else {
        return lvalue;
    }
}

int term() {
    int lvalue = factor();
    return term_tail(lvalue);
}

int expr_tail(int lvalue) {
    if (token == '+') {
        match('+');
        int value = lvalue + term();
        return expr_tail(value);
    } else if (token == '-') {
        match('-');
        int value = lvalue - term();
        return expr_tail(value);
    } else {
        return lvalue;
    }
}

int expr() {
    int lvalue = term();
    return expr_tail(lvalue);
}
```

Implmenting a top-down parser is straightforward with the help of BNF grammar.
We now add the code for lexer:

```c
#include <stdio.h>
#include <stdlib.h>

enum {Num};
int token;
int token_val;
char *line = NULL;
char *src = NULL;

void next() {
    // skip white space
    while (*src == ' ' || *src == '\t') {
        src ++;
    }
    
    token = *src++;
    
    if (token >= '0' && token <= '9' ) {
        token_val = token - '0';
        token = Num;
        
        while (*src >= '0' && *src <= '9') {
            token_val = token_val*10 + *src - '0';
            src ++;
        }
        return;
    }
}

void match(int tk) {
    if (token != tk) {
        printf("expected token: %d(%c), got: %d(%c)\n", tk, tk, token, token);
        exit(-1);
    }
    
    next();
}
```

Finally, the `main` method to bootstrap:

```c
int main(int argc, char *argv[])
{
    size_t linecap = 0;
    ssize_t linelen;
    while ((linelen = getline(&line, &linecap, stdin)) > 0) {
        src = line;
        next();
        printf("%d\n", expr());
    }
    return 0;
}
```

You can play with your own calculator now. Or try to add some more functions
based on what we've learned in the previous chapter. Such as variable support
so that user can define variable to store values.

## Summary

We don't like theory, but it exists for good reason as you can see that BNF can
help us to build the parser. So I want to convice you to learn some theories,
it will help you to become a better programmer.

Top-down parsing technique is often used in manually crafting of parsers, so
you are able to handle most jobs if you master it! As you'll see in laster
chapters.
