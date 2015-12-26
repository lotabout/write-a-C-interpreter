# How to Run the Code

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

Also I write a [series of
article](http://lotabout.github.io/2015/%E5%86%99%E4%B8%AA-C-%E7%BC%96%E8%AF%91%E5%99%A8-1/)
about how this compiler is built(in Chinese though).

# Licence

The original code is licenced with GPL2, so this code will use the same
licence.
