This series of articles is a tutorial for building a C compiler from scratch.

I lied a little in the above sentence: it is actually an _interpreter_ instead
of a _compiler_. I lied because what the hell is a "C interpreter"? You will
however, understand compilers better by building an interpreter.

Yeah, I wish you can get a basic understanding of how a compiler is
constructed, and realize it is not that hard to build one. Good Luck!

Finally, this series is written in Chinese in the first place, feel free to
correct me if you are confused by my English. And I would like it very much if
you could teach me some "native" English :)

We won't write any code in this chapter, so feel free to skip it if you are
desperate to see some code...

## Why you should care about compiler theory?

Because it is **COOL**!

And it is very useful. Programs are built to do things for us, and when they translate some forms of data into another form, we can call them compilers. Thus, by learning some compiler theory, we are mastering a very
powerful technique of solving problems. Isn't that cool?

People used to say that understanding how a compiler works would help you write
better code. Some would argue that modern compilers are so good at
optimization that you should not care any more. Well, that's true, since most people
don't need to learn compiler theory only to improve the efficency of the code.
And by most people, I mean you!

## We Don't Like Theory Either

I have always been in awe of compiler theory because that's what makes
programming easy. Can you imagine building a web browser only in Assembly? When I got a chance to learn compiler theory in college,
I was so excited! And then... I quit, not understanding it...

Normally a compiler course will cover:

1. How to represent syntax (such as BNF, etc.)
2. Lexers, with some NFA (Nondeterministic Finite Automata),
   DFA (Deterministic Finite Automata).
3. Parsers, such as recursive descent, LL(k), LALR, etc.
4. Intermediate Languages.
5. Code generation.
6. Code optimization.

Perhaps more than 90% of students will not care about anything beyond the parser, and
what's more, we still don't know how to build a compiler! Even after all the
effort learning the theories. The main reason is that what "Compiler
Theoy" tries to teach is "How to build a parser generator", namely a tool that
consumes syntax grammer and generates a compiler for you, like lex/yacc or
flex/bison and other things like that.

These theories try to teach us how to solve the general problems of generating
compilers automatically. That means once you've mastered them, you are able to
deal with all kinds of grammars. They are indeed useful in industry.
Nevertheless they are too powerful and too complicated for students and most
programmers. You will understand that if you try to read lex/yacc's source
code.

The good news is building a compiler is much simpler than you can imagine.
I won't lie, it's not easy, but definitely not hard.

## Birth of this project

One day I came across the project [c4](https://github.com/rswier/c4) on
Github. It is a small C interpreter which is claimed to be implemented by only
4 functions. The most amazing part is that it is bootstrapping (it interprets
itself), and it is only done with about 500 lines!

Meanwhile I've read a lot of tutorials about compiler, they are either too
simple (like implementing a simple calculator) or using automation
tools (like flex/bison). C4, however, is implemented from scratch. The
sad thing is that it tries to be minimal, making the code quite a mess, and hard
to understand. So I started a new project that would:

1. Implement a working C compiler(interpreter actually)
2. Write a tutorial of how it is built.

It took me 1 week to re-write it, resulting in 1400 lines including comments. The
project is hosted on Github: [Write a C Interpreter](https://github.com/lotabout/write-a-C-interpreter).

Thanks rswier for bringing us a wonderful project!

## Before you go

Implementing a compiler can be boring and it is hard to debug. I hope you
can spare enough time studying, as well as type the code. I am sure that you
will feel a great sense of accomplishment just like I did.

## Good Resources

1. [Let’s Build a Compiler](http://compilers.iecc.com/crenshaw/): a very good
   tutorial of building a compiler for fresh starters.
2. [Lemon Parser Generator](http://www.hwaci.com/sw/lemon/): the parser
   generator that is used in SQLite. Good to read if you want to understand
   compiler theory with code.

In the end, I am human at a general level, so there will inevitably be errors
with the articles and code (also my English). Feel free to correct me!

Hope you enjoy.
