> lexical analysis is the process of converting a sequence of characters (such
> as in a computer program or web page) into a sequence of tokens (strings with
> an identified "meaning").

Normally we represent the token as a pair: `(token type, token value)`. For
example, if a program's source file contains string: "998", the lexer will
treat it as token `(Number, 998)` meaning it is a number with value of `998`.


## Lexer vs Compiler

Let's first look at the structure of a compiler:

```
                   +-------+                      +--------+
-- source code --> | lexer | --> token stream --> | parser | --> assembly
                   +-------+                      +--------+
```

The Compiler can be treated as a transformer that transform C source code into
assembly. In this sense, lexer can parser are transformers as well: Lexer
takes C source code as input and output token stream; Parser will consume the
token stream and generate assembly code.

Then why do we need lexer and a parser? Well the Compiler's job is hard! So we
recruit lexer to do part of the job and parser to do the rest so that each
will need to deal with simple one only.

That's the value of a lexer: to simplify the parser by converting the stream
of source code into token stream.

## Implementation Choice

Before we start I want to let you know that crafting a lexer is boring and
error-prone. That's why geniuses out there had already created automation
tools to do the job. `lex/flex` are example that allows us to describe the
lexical rules using regular expressions and generate lexer for us.

Also note that we won't follow the graph in the section above, i.e. not
converting all source code into token stream at once. The reasons are:

1. Converting source code into token stream is stateful. How a string is
   interpreted is related with the place where it appears.
2. It is a waste to store all the tokens because only a few of them will be
   accessed at the same time.

Thus we'll implement one function: `next()` that returns a token in one call.

## Tokens Supported

Add the definition into the global area:

```c
// tokens and classes (operators last and in precedence order)
enum {
  Num = 128, Fun, Sys, Glo, Loc, Id,
  Char, Else, Enum, If, Int, Return, Sizeof, While,
  Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak
};
```

These are all the tokens our lexer can understand. Our lexer will interpret
string `=` as token `Assign`; `==` as token `Eq`; `!=` as `Ne`; etc.

So we have the impression that a token will contain one or more characters.
That is the reason why lexer can reduce the complexity, now the parser doesn't
have to look at several character to identify a the meaning of a substring.
The job had been done.

Of course, the tokens above is properly ordered reflecting their priority in
the C programming language. `*(Mul)` operator for example has higher priority
the `+(Add)` operator. We'll talk about it later.

At last, there are some characters we don't included here are themselves a
token such as `]` or `~`. The reason that we done encode them like others are:

1. These tokens contains only single character, thus are easier to identify.
2. They are not involved into the priority battle.

## Skeleton of Lexer

```c
void next() {
    char *last_pos;
    int hash;
    while (token = *src) {
        ++src;
        // parse token here
    }
    return;
}
```

While do we need `while` here knowing that `next` will only parse one token?
This raises a quesion in compiler construction(remember that lexer is kind of
compiler?): How to handle error?

Normally we had two solutions:

1. points out where the error happans and quit.
2. points out where the error happans, skip it, and continue.

That will explain the existance of `while`: to skip unknown characters in the
source code. Meanwhile it is used to skip whitespaces which is not the actual
part of a program. They are treated as separators only.

## Newline

It is quite like space in that we are skipping it. The only difference is that
we need to increase the line number once a newline character is met:

```c
// parse token here
...
if (token == '\n') {
    ++line;
}
...
```

## Macros

Macros in C starts with character `#` such as `#include <stdio.h>`. Our
compiler don't support any macros, so we'll skip all of them:

```c
else if (token == '#') {
    // skip macro, because we will not support it
    while (*src != 0 && *src != '\n') {
        src++;
    }
}
```

## Identifers and Symbol Table

Identifier is the name of a variable. But we don't actually care about the
names in lexer, we cares about the identity. For example: `int a;`
declares a variable, we have to know that the statement `a = 10` that comes
after refers to the same variable that we declared before.

Based on this reason, we'll make a table to store all the names we've already
met and call it Symbol Table. We'll look up the table when a new
name/identifier is accountered. If the name exists in the symbol table, the
identity is returned.

Then how to represent an identity?

```c
struct identifier {
    int token;
    int hash;
    char * name;
    int class;
    int type;
    int value;
    int Bclass;
    int Btype;
    int Bvalue;
}
```

We'll need a little explanation here:

1. `token`: is the token type of an identifier. Theoretically it should be
   fixed to type `Id`. But it is not true because we will add keywords(e.g
   `if`, `while`) as special kinds of identifier.
2. `hash`: the hash value of the name of the identifier, to speed up the
   comparision of table lookup.
3. `name`: well, name of the identifier.
4. `class`: Whether the identifier is global, local or constants.
5. `type`: type of the identifier, `int`, `char` or pointer.
6. `value`: the value of the variable that the identifier points to.
7. `BXXX`: local variable can shadow global variable. It is used to store
   global ones if that happens.

Traditional symbol table will contain only the unique identifer while our
symbol table stores other information that will only be accessed by parser
such as `type`.

Yet sadly, our compiler do not support `struct` while we are trying to be
bootstrapping. So we have to compromise in the actual structure of an
identifier:

```
Symbol table:
----+-----+----+----+----+-----+-----+-----+------+------+----
 .. |token|hash|name|type|class|value|btype|bclass|bvalue| ..
----+-----+----+----+----+-----+-----+-----+------+------+----
    |<---       one single identifier                --->|
```

That means we use a single `int` array to store all identifier information.
Each ID will use 9 cells. The code is as following:

```c
int token_val;                // value of current token (mainly for number)
int *current_id,              // current parsed ID
    *symbols;                 // symbol table
// fields of identifier
enum {Token, Hash, Name, Type, Class, Value, BType, BClass, BValue, IdSize};
void next() {
        ...
        else if ((token >= 'a' && token <= 'z') || (token >= 'A' && token <= 'Z') || (token == '_')) {
            // parse identifier
            last_pos = src - 1;
            hash = token;
            while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') || (*src == '_')) {
                hash = hash * 147 + *src;
                src++;
            }
            // look for existing identifier, linear search
            current_id = symbols;
            while (current_id[Token]) {
                if (current_id[Hash] == hash && !memcmp((char *)current_id[Name], last_pos, src - last_pos)) {
                    //found one, return
                    token = current_id[Token];
                    return;
                }
                current_id = current_id + IdSize;
            }
            // store new ID
            current_id[Name] = (int)last_pos;
            current_id[Hash] = hash;
            token = current_id[Token] = Id;
            return;
        }
        ...
}
```

Note that the search in symbol table is linear search.

## Number

We need to support decimal, hexadecimal and octal. The logic is quite
straightforward except how to get the hexadecimal value. Maybe..

```c
token_val = token_val * 16 + (token & 0x0F) + (token >= 'A' ? 9 : 0);
```

In case you are not familiar with this "small trick", `a`'s hex value is `61`
in ASCII while `A` is `41`. Thus `token & 0x0F` can get the the smallest digit
out of the character.

```c
void next() {
        ...



        else if (token >= '0' && token <= '9') {
            // parse number, three kinds: dec(123) hex(0x123) oct(017)
            token_val = token - '0';
            if (token_val > 0) {
                // dec, starts with [1-9]
                while (*src >= '0' && *src <= '9') {
                    token_val = token_val*10 + *src++ - '0';
                }
            } else {
                // starts with number 0
                if (*src == 'x' || *src == 'X') {
                    //hex
                    token = *++src;
                    while ((token >= '0' && token <= '9') || (token >= 'a' && token <= 'f') || (token >= 'A' && token <= 'F')) {
                        token_val = token_val * 16 + (token & 15) + (token >= 'A' ? 9 : 0);
                        token = *++src;
                    }
                } else {
                    // oct
                    while (*src >= '0' && *src <= '7') {
                        token_val = token_val*8 + *src++ - '0';
                    }
                }
            }
            token = Num;
            return;
        }


        ...
}
```


## String Literals

If we found any string literal, we need to store it into the `data segment`
that we introduced in previous chapter and return the address. Another issue
is we need to care about escaped characters such as `\n` to represent newline
character. But we don't support escaped characters other than `\n` like `\t`
or `\r`because we aims at bootstrapping only. Note that we still support
syntax that `\x` to be character `x` itself.

Our lexer will analyze single character (e.g. `'a'`) at the same time. Once
character is found, we return it as a `Num`.

```c
void next() {
        ...

        else if (token == '"' || token == '\'') {
            // parse string literal, currently, the only supported escape
            // character is '\n', store the string literal into data.
            last_pos = data;
            while (*src != 0 && *src != token) {
                token_val = *src++;
                if (token_val == '\\') {
                    // escape character
                    token_val = *src++;
                    if (token_val == 'n') {
                        token_val = '\n';
                    }
                }
                if (token == '"') {
                    *data++ = token_val;
                }
            }

            src++;
            // if it is a single character, return Num token
            if (token == '"') {
                token_val = (int)last_pos;
            } else {
                token = Num;
            }

            return;
        }
}
```

## Comments

Only C++ style comments(e.g. `// comment`) is supported. C style (`/* ... */`)
is not supported.

```c
void next() {
        ...

        else if (token == '/') {
            if (*src == '/') {
                // skip comments
                while (*src != 0 && *src != '\n') {
                    ++src;
                }
            } else {
                // divide operator
                token = Div;
                return;
            }
        }

        ...
}
```

Now we'll introduce the concept: `lookahead`. In the above code we see that
for source code starting with character `/`, either 'comment' or `/(Div)` may
be encountered.

Sometimes we cannot decide which token to generate by only looking at current
character(such as the above example about divide and comment), thus we need to
check the next character(called `lookahead`) in order to determine. In our
example, if it is another slash `/`, then we've encountered a comment line,
otherwise it is a divide operator.

Like we've said that lexer and parser are inherently some kind of compiler,
`lookahead` also exists in parser. However parser will look ahead for "token"
instead of "character". The `k` in `LL(k)` of compiler theory is the amount of
tokens a parser needs to look ahead.

Also if we don't split compiler into lexer and parser, the compiler will have
to look ahead a lot of character to decide what to do next. So we can say that
lexer reduced the amount of lookahead a compiler need to check.

## Others

Others are simpiler and straightforward, check the code:

```c
void next() {
        ...
        else if (token == '=') {
            // parse '==' and '='
            if (*src == '=') {
                src ++;
                token = Eq;
            } else {
                token = Assign;
            }
            return;
        }
        else if (token == '+') {
            // parse '+' and '++'
            if (*src == '+') {
                src ++;
                token = Inc;
            } else {
                token = Add;
            }
            return;
        }
        else if (token == '-') {
            // parse '-' and '--'
            if (*src == '-') {
                src ++;
                token = Dec;
            } else {
                token = Sub;
            }
            return;
        }
        else if (token == '!') {
            // parse '!='
            if (*src == '=') {
                src++;
                token = Ne;
            }
            return;
        }
        else if (token == '<') {
            // parse '<=', '<<' or '<'
            if (*src == '=') {
                src ++;
                token = Le;
            } else if (*src == '<') {
                src ++;
                token = Shl;
            } else {
                token = Lt;
            }
            return;
        }
        else if (token == '>') {
            // parse '>=', '>>' or '>'
            if (*src == '=') {
                src ++;
                token = Ge;
            } else if (*src == '>') {
                src ++;
                token = Shr;
            } else {
                token = Gt;
            }
            return;
        }
        else if (token == '|') {
            // parse '|' or '||'
            if (*src == '|') {
                src ++;
                token = Lor;
            } else {
                token = Or;
            }
            return;
        }
        else if (token == '&') {
            // parse '&' and '&&'
            if (*src == '&') {
                src ++;
                token = Lan;
            } else {
                token = And;
            }
            return;
        }
        else if (token == '^') {
            token = Xor;
            return;
        }
        else if (token == '%') {
            token = Mod;
            return;
        }
        else if (token == '*') {
            token = Mul;
            return;
        }
        else if (token == '[') {
            token = Brak;
            return;
        }
        else if (token == '?') {
            token = Cond;
            return;
        }
        else if (token == '~' || token == ';' || token == '{' || token == '}' || token == '(' || token == ')' || token == ']' || token == ',' || token == ':') {
            // directly return the character as token;
            return;
        }

        ...
}
```

## Keywords and Builtin Functions

Keywords such as `if`, `while` or `return` are special because they are known
by the compiler in advance. We cannot treat them like normal identifiers
because the special meanings in it. There are two ways to deal with it:

1. Let lexer parse them and return a token to identify them.
2. Treat them as normal identifier but store them into the symbol table in
   advance.

We choose the second way: add corresponding identifers into symbol table in
advance and set the needed properties(e.g. the `Token` type we mentioned). So
that when keywords are encountered in the source code, they will be interpret
as identifiers, but since they already exists in the symbol table we can know
that they are different with normal identifiers.

Builtin function are similar. They are only different in the internal
information. In the main function, add the following:

```c
// types of variable/function
enum { CHAR, INT, PTR };
int *idmain;                  // the `main` function
void main() {
    ...

    src = "char else enum if int return sizeof while "
          "open read close printf malloc memset memcmp exit void main";


    // add keywords to symbol table
    i = Char;
    while (i <= While) {
        next();
        current_id[Token] = i++;
    }

    // add library to symbol table
    i = OPEN;
    while (i <= EXIT) {
        next();
        current_id[Class] = Sys;
        current_id[Type] = INT;
        current_id[Value] = i++;
    }

    next(); current_id[Token] = Char; // handle void type
    next(); idmain = current_id; // keep track of main

    ...
    program();
}
```

## Code

You can check out the code on
[Github](https://github.com/lotabout/write-a-C-interpreter/tree/step-2), or
clone with:

```
git clone -b step-2 https://github.com/lotabout/write-a-C-interpreter
```

Executing the code will give 'Segmentation Falt' because it will try to
execute the virtual machine that we build in previous chapter which will not
work because it doesn't contain any runnable code.

## Summary

1. Lexer is used to pre-process the source code, so as to reduce the
   complexity of parser.
2. Lexer is also a kind of compiler which consumes source code and output
   token stream.
3. `lookahead(k)` is used to fully determine the meaning of current
   character/token.
4. How to represent identifier and symbol table.

We will discuss about top-down recursive parser. See you then :)
