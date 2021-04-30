# Preface

This is multi-part tutorial on how to build a C compiler from scratch.

Well, I lied a little in the previous sentence: it's actually an _interpreter_,
not a _compiler_. I had to lie, because what on earth is a "C interpreter"?
You will however gain a better understanding of compilers by building an
interpreter.

Yeah, I want to provide you with a basic understanding of how a compiler is
constructed, and realize that it's not that hard to build one, after all.
Good Luck!

This tutorial was originally written in Chinese, so feel free to correct me if
you're confused by my English. Also, I would really appreciate it if you could
teach me some "native" English. :smile:

We won't be writing any code in this chapter; so if you're eager to see some code, feel free to skip it.


## Why Should I Care about Compiler Theory?

Because it's **COOL**!

And it's also very useful. Programs are designed to do something for us; when
they are used to translate some form of data into another form, we can call
them compilers. Thus, by learning some compiler theory, we are trying to
master a very powerful problem solving technique. Doesn't this sound cool
enough to you?

People used to say that understanding how a compiler works would help you to
write better code. Some would argue that modern compilers are so good at
optimizing that you shouldn't care any more. Well, that's true, most people
don't need to learn compiler theory to improve code performance — and by "most
people" I mean _you_!


## We Don't Like Theory Either

I've always been in awe of compiler theory because that's what makes programing
easy. Anyway, can you imagine building a web browser entirely in assembly
language? So when I got a chance to learn compiler theory in college, I was so
excited! And then ... I quit! And left without understanding what it's all
about.

Normally compiler course covers the following topics:

1. How to represent syntaxes (i.e. BNF, etc.)
2. Lexers, using NFA (Nondeterministic Finite Automata) and
   DFA (Deterministic Finite Automata).
3. Parsers, such as recursive descent, LL(k), LALR, etc.
4. Intermediate Languages.
5. Code generation.
6. Code optimization.

Perhaps more than 90% of the students won't really care about any of that,
except for the parser, and what's more, we'd still won't know how to actually
build a compiler! even after all the effort of learning the theory. Well, the
main reason is that what "Compiler Theory" tries to teach is "how to build a
parser generator" — i.e. a tool that consumes a syntax grammar and generates a
compiler for you, like lex/yacc or flex/bison, or similar tools.

These theories try to teach us how to solve the general challenges of
generating compilers automatically. Once you've mastered them, you're able to
deal with all kinds of grammars. They are indeed useful in the industry.
Nevertheless, they are too powerful and too complicated for students and most
programmers. If you try to read lex/yacc's source code you'll understand what
I mean.

The good news is that building a compiler can be much simpler than you ever
imagined. I won't lie, it's not easy, but definitely not hard.


## How This Project Began

One day I came across the project [c4] on Github, a small C interpreter
claiming to be implemented with only 4 functions. The most amazing part is
that it's [bootstrapping]  (i.e. it can interpret itself). Furthermore, it's
being done in around 500 lines of code!

Meanwhile, I've read many tutorials on compilers design, and found them to be
either too simple (such as implementing a simple calculator) or using
automation tools (such as flex/bison). [C4], however, is implemented entirely
from scratch. The sad thing is that it aims to be "an exercise in minimalism,"
which makes the code quite messy and hard to understand. So I started a new
project, in order to:

1. Implement a working C compiler (an interpreter, actually).
2. Write a step-by-step tutorial on how it was built.

It took me one week to re-write it, resulting in 1400 lines of code (including
comments). The project is hosted on Github: [Write a C Interpreter].

Thanks [@rswier] for sharing with us [c4], it's such a wonderful project!


## Before You Begin

Implementing a compiler can be boring and hard to debug. So I hope you can
spare enough time studying, and typing code. I'm sure that you will feel a
great sense of accomplishment, just like I do.


## Good Resources

1. _[Let’s Build a Compiler]_: a very good tutorial of building a compiler,
    written for beginners.
2. [Lemon Parser Generator]: the parser generator used by SQLite.
   Good to read if you want to understand compiler theory with code.

In the end, I am just a person with a general level of expertise, so there
will inevitably be some mistakes in my articles and code (and also in my
English). Feel free to correct me!

I hope you'll enjoy it.

<!-----------------------------------------------------------------------------
                               REFERENCE LINKS
------------------------------------------------------------------------------>

[@rswier]: https://github.com/rswier "Visit @rswier's GitHub profile"
[bootstrapping]: https://en.wikipedia.org/wiki/Bootstrapping_(compilers) "Wikipedia » Bootstrapping (compilers)"
[c4]: https://github.com/rswier/c4 "Visit the c4 repository on GitHub"
[Lemon Parser Generator]: http://www.hwaci.com/sw/lemon/ "Visit Lemon homepage"
[Let’s Build a Compiler]: http://compilers.iecc.com/crenshaw/ "15-part tutorial series, by Jack Crenshaw"
[Write a C Interpreter]: https://github.com/lotabout/write-a-C-interpreter "Visit the 'Write a C Interpreter' repository on GitHub"
