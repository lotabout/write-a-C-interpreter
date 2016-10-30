We have to concept in C: statement and expression. Basically statements won't
have a value as its result while expressions do. That means you cannot assign
an statement to a variable.

We have 6 statements in our interpreter:

1. `if (...) <statement> [else <statement>]`
2. `while (...) <statement>`
3. `{ <statement> }`
4. `return xxx;`
5. `<empty statement>`;
6. `expression;` (expression end with semicolon)

The parsing job is relatively easy compared to understanding the expected
assembly output. Let's explain one by one.

## IF

`if` statement is to jump to a different location according to the condition.
Let's check its pseudo cdoe:

```
if (...) <statement> [else <statement>]

  if (<cond>)                   <cond>
                                JZ a
    <true_statement>   ===>     <true_statement>
  else:                         JMP b
a:                           a:
    <false_statement>           <false_statement>
b:                           b:
```

The flow of assembly code is:

1. execute `<cond>`.
2. If the condition fail, jump to positino `a`, i.e. `else` statement.
3. Cause assembly is executed sequentially, if `<true_statement>` is executed,
   we need to skip `<false_statement>`, thus a jump to `b` is needed.

Corresponding C code:

```c
    if (token == If) {
        match(If);
        match('(');
        expression(Assign);  // parse condition
        match(')');

        *++text = JZ;
        b = ++text;

        statement();         // parse statement
        if (token == Else) { // parse else
            match(Else);

            // emit code for JMP B
            *b = (int)(text + 3);
            *++text = JMP;
            b = ++text;

            statement();
        }

        *b = (int)(text + 1);
    }
```

## While

`while` is simplier than `if`:

```
a:                     a:
   while (<cond>)        <cond>
                         JZ b
    <statement>          <statement>
                         JMP a
b:                     b:
```

Nothing worth mention. C code:

```c
    else if (token == While) {
        match(While);

        a = text + 1;

        match('(');
        expression(Assign);
        match(')');

        *++text = JZ;
        b = ++text;

        statement();

        *++text = JMP;
        *++text = (int)a;
        *b = (int)(text + 1);
    }
```

## Return

Once we met `return`, it means the function is about to end, thus `LEV` is
needed to indicate the exit.

```c
    else if (token == Return) {
        // return [expression];
        match(Return);

        if (token != ';') {
            expression(Assign);
        }

        match(';');

        // emit code for return
        *++text = LEV;
    }
```

## Others

Other statement acts as helpers for compiler to group the codes better. They
won't generate assembly codes. As follows:

```c
    else if (token == '{') {
        // { <statement> ... }
        match('{');

        while (token != '}') {
            statement();
        }

        match('}');
    }
    else if (token == ';') {
        // empty statement
        match(';');
    }
    else {
        // a = b; or function_call();
        expression(Assign);
        match(';');
    }
```

## Code

You can download the code of this chapter from [Github](https://github.com/lotabout/write-a-C-interpreter/tree/step-5), or clone with:

```
git clone -b step-5 https://github.com/lotabout/write-a-C-interpreter
```

The code still won't run because there are still some un-implemented
functions. You can challange yourself to fill them out first.

## Summary

As you can see, implementing parsing for an interpreter is not hard at all.
But it did seems complicated because we need to gather enough knowledge during
parsing in order to generate target code(assembly in our case). You see, that
is one big obstacle for beginner to start implementation. So instead of
"programming knowledge", "domain knowledge" is also required to actually
achieve something.

Thus I suggest you to learn assembly if you haven't, it is not difficult but
helpful to understand how computers work.
