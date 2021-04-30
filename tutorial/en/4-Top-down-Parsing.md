# 4. Top-down Parsing

In this chapter we will build a simple calculator using the [top-down parsing]
technique. This is just preparation work, before we start to implement the
parser.

I will introduce a small set of theories, but can't guarantee them to be
absolutely correct, please consult your textbook if you have any confusion.


## Top-down Parsers

Traditionally, we have top-down parsing and [bottom-up parsing]. The top-down
method will start with a non-terminator and recursively check the source code
to replace every non-terminators with its alternatives, until no
non-terminator is left.

You see I used the top-down method for explaining "top-down" because you'll
have to know what a "non-terminator" is to understand the previous paragraph.
But I haven't told you what that is. We will explain it in the next section.
For now, consider "top-down" as trying to tear down a big object into small
pieces.

On the other hand, "bottom-up" parsing is trying to combine small objects into
a bigger one. It is often used in automation tools that generate parsers.


## Terminators and Non-terminators

They are terms used in [BNF]  (Backus–Naur Form), which is a language used to
describe grammars. A simple elementary arithmetic calculator in BNF will be:

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

Item enclosed by `<>` are called *Non-terminators*. They got that name because
we can replace them with the items on the right hand side of `::=`. `|` means
alternative, i.e. you can replace `<term>` with any one of
`<term> * <factor>`, `<term> / <factor>` or `<factor>`.
Those which do not appear on the left side of `::=` are called *Terminators*,
such as `+`, `(`, `Num`, etc. They often corresponds to the tokens we get from
the lexer.


## Top-down Example of a Simple Calculator

The parse tree is the inner structure we get after the parser consumes all the
tokens and finishes all the parsing. Let's take `3 * (4 + 2)` as an example to
show the connections between BNF grammar, parse tree and top-down parsing.

Top-down parsing beings from a starting non-terminator, which is `<term>` in
our example. You can specify it in real practice, but also defaults to the
first non-terminator encountered.

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
alternatives (top-down) until all of the sub-items are replaced by
terminators (bottom). Some non-terminators are used recursively such as
`<expr>`.


## Advantages of Top-down Parsing

As you can see in the above example, the parsing steps are similar to the BNF
grammar. Which means it is easy to convert the grammar into actual code by
converting a production rule (`<...> ::= ...`) into a function with the same
name.

One question arises here: how do you know which alternative to apply? Why do
you choose `<expr> ::= <term> * <factor>` over `<expr> ::= <term> / <factor>`?
That's right, with `lookahead`! We peek the next token, which is `*`, so it's
the first one to apply.

However, top-down parsing requires that the grammar doesn't contain
left-recursion.


## Left-recursion

Suppose we have a grammar like this:

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

As you can see, function `expr` will never exit! In the grammar, the `<expr>`
non-terminator is used recursively and appears immediately after `::=`, which
causes left-recursion.

Luckily, most left-recursive grammars (maybe all? I don't remember) can be
properly transformed into non left-recursive equivalents. Our calculator
grammar can be converted into:

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

Implementing a top-down parser is straightforward with the help of a BNF
grammar.

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
based on what we've learned in the previous chapter. Such as variable support,
so that a user can define variables to store values.


## Summary

We don't like theory, but it exists for a good reason as you can see that BNF
can help us to build the parser. So I want to convince you to learn some
theory, it will help you to become a better programmer.

The top-down parsing technique is often used when crafting parsers manually,
so you'll be able to handle most jobs if you master it! As you'll see in later
chapters.

<!-----------------------------------------------------------------------------
                               REFERENCE LINKS
------------------------------------------------------------------------------>

[BNF]: https://en.wikipedia.org/wiki/Backus%E2%80%93Naur_Form "Wikipedia: Backus–Naur form"
[bottom-up parsing]: https://en.wikipedia.org/wiki/Bottom-up_parsing "Wikipedia » Bottom-up parsing"
[top-down parsing]: https://en.wikipedia.org/wiki/Top-down_parsing "Wikipedia » Top-down parsing"
