C interpreter that interpretes itself.

# How to Run the Code

File `xc.c` is the original one and `xc-tutor.c` is the one that I make for
the tutorial step by step.

```
gcc -o xc xc.c (you may need the -m32 option on 64bit machines)
./xc hello.c
./xc -s hello.c

./xc c4.c hello.c
./xc c4.c c4.c hello.c
```

# About

This project is inspired by [c4](https://github.com/rswier/c4) and is largely
based on it.

However, I rewrited them all to make it more understable and help myself to
understand it.

Despite the complexity we saw in books about compiler design, writing one is
not that hard. You don't need that much theory though they will help for
better understanding the logic behind the code.

Also I write a series of article about how this compiler is built(in Chinese though):

1. [手把手教你构建 C 语言编译器（0）——前言](http://lotabout.me/2015/write-a-C-interpreter-0/)
2. [手把手教你构建 C 语言编译器（1）——设计](http://lotabout.me/2015/write-a-C-interpreter-1/)
3. [手把手教你构建 C 语言编译器（2）——虚拟机](http://lotabout.me/2015/write-a-C-interpreter-2/)
4. [手把手教你构建 C 语言编译器（3）——词法分析器](http://lotabout.me/2015/write-a-C-interpreter-3/)
4. [手把手教你构建 C 语言编译器（4）——递归下降](http://lotabout.me/2016/write-a-C-interpreter-4/)
5. [手把手教你构建 C 语言编译器（5）——变量定义](http://lotabout.me/2016/write-a-C-interpreter-5/)
6. [手把手教你构建 C 语言编译器（6）——函数定义](http://lotabout.me/2016/write-a-C-interpreter-6/)
7. [手把手教你构建 C 语言编译器（7）——语句](http://lotabout.me/2016/write-a-C-interpreter-7/)
8. [手把手教你构建 C 语言编译器（8）——表达式](http://lotabout.me/2016/write-a-C-interpreter-8/)
0. [手把手教你构建 C 语言编译器（9）——总结](http://lotabout.me/2016/write-a-C-interpreter-9/)

# Resources

Further Reading:

- [Let's Build a Compiler](http://compilers.iecc.com/crenshaw/): An excellent
    starting material for building compiler.


Forks:

- [A fork that implement debugger for xc.c](https://github.com/descent/write-a-C-interpreter)


# Licence

The original code is licenced with GPL2, so this code will use the same
licence.
