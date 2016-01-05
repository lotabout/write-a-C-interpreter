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

- [手把手教你构建 C 语言编译器（0）](http://lotabout.me/2015/write-a-C-interpreter-0/)
- [手把手教你构建 C 语言编译器（1）](http://lotabout.me/2015/write-a-C-interpreter-1/)
- [手把手教你构建 C 语言编译器（2）](http://lotabout.me/2015/write-a-C-interpreter-2/)
- [手把手教你构建 C 语言编译器（3）](http://lotabout.me/2015/write-a-C-interpreter-3/)
- [手把手教你构建 C 语言编译器（4）](http://lotabout.me/2015/write-a-C-interpreter-4/)
- [手把手教你构建 C 语言编译器（5）](http://lotabout.me/2015/write-a-C-interpreter-5/)
- [手把手教你构建 C 语言编译器（6）](http://lotabout.me/2015/write-a-C-interpreter-6/)
- [手把手教你构建 C 语言编译器（7）](http://lotabout.me/2015/write-a-C-interpreter-7/)
- [手把手教你构建 C 语言编译器（8）](http://lotabout.me/2015/write-a-C-interpreter-8/)
- [手把手教你构建 C 语言编译器（9）](http://lotabout.me/2015/write-a-C-interpreter-9/)

# Licence

The original code is licenced with GPL2, so this code will use the same
licence.
