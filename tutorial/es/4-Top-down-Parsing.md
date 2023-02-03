En este capítulo vamos a contruir una simple calculadora utilizando top-down
parsing. Esta es la preparación antes de construir nuestro parser.

Introduciré algunas teorías pero no garantizo estar en lo corrector. Porfavor,
consulta una funte veraz si tienes alguna confusión.

## Top-down parsing

Tradicionálmente hemos tenido top-down y bottom-up parsing. Top-down empieza
con un no terminal y recursivamente compruba el código para reemplazar los
no terminales con alternativas hasta que no quede ninguno.

Necesitaras saber lo que es un `no terminal` para entender el parágro previo.
Pero no te he dicho lo que es. Lo explicaré en la siguiente sección. Por ahora
considera que `top-down` intenta partir un gran proyecto en pequeñas piezas.

Por otra parte `bottom-up` parsing intenta combinar pequeños objetos en uno
grande. Se usa amenudo en herramientas de automatización para generar parsers.

## Terminal y no terminal

Son terminos utilizados en
[BNF](https://es.wikipedia.org/wiki/Notación_de_Backus-Naur) (Notación
Backus-Naur) el cual es un lenguaje utilizado para describir gramática.
Una calculadora simple en BNF seria:

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

Los items entre `<>` se llaman `no terminales`. Tienen ese nombre porque
podemos reemplazarlos con los items a la derecha de `::=`. `|` significa
una alternativa para reemplazar el no terminal. Aquellos que no aparecen
a la izquierda de `::=` se denominan `terminales`, como lo son `()` o `Num`.

## Ejemplo de un top-down para una simple calculadora

El arbol de parseo es la estructura interna que obtenemos despues de que el
parser consuma todos los tokens y termine de hacer todo el parsing. Tomemos
`3 * (4 + 2)` como ejemplo para mostrar las conexiones en la gramática BNF,
arbol de parse, y top-down parsing.

Top-down parsing empieza desde un no terminal, en nuestro caso es `<term>`.
Puedes especificar por cual se empiza, o si no sera el primero que se encuentre.

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

Puedes ver que cada paso cambia un no terminal usando una de sus alternativas
hasta que todos los items son sustituidos por teminales. Algunos no terminales
son utilizados recursivamente, como por ejemplo `<expr>`.

## Ventajas de Top-down parsing

Como puedes ver en el ejemplo de arriba el paso de parsing es similar a la
gramática BNF. Lo cual significa que es fácil convertir la gramática en código
convirtiendo una regla (`<...> ::= ...`) en una función con el mismo nombre.

Tenemos una duda: ¿Como sabemos que alternativa usar? ¿Porque elejir
`<expr> ::= <term> * <factor>` y no `<expr> ::= <term> / <factor>`?
Para eso tenemos `lookahead`. Miramos el siguiente token y así sabremos
cual alternativa aplicar.

Sin embargo, top-down parsing requiere que la gramática no tenga recursion por
la izquierda.

## Recursión por la izquierda

Supongamos que tenemos la siguiente gramática:

```
<expr> ::= <expr> + Num
```

lo podemos traducir en la siguiente función:

```
int expr() {
    expr();
    num();
}
```	

Como puedes ver, la función `expr` nunca terminara. En la gramática se utiliza
`<expr>` inmediatamente de forma recursiva, lo que causa recursiónpor la
izquierda.

Afortunadamente, la mayoría de las gramáticas recursivas por la izquierda
(tal vez todas, no recurdo bien) pueden ser transformadas en equivalentes
que no sean recursivas por la izquierda. Nuestra gramática para la calculadora
puede ser transformada en lo siguiente:

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

Si deseas más información deberias de buscarla en otra fuente.

## Implementación

El código es directamente convertido desde la gramática. Notese como de
sencillo es:

```c
int expr();
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

Implementar un top-down parser es sencillo con la ayuda de una gramática BNF.
Ahora ayadimos el core del lexer:

```c
#include <stdio.h>
#include <stdlib.h>

enum {Num};
int token;
int token_val;
char *line = NULL;
char *src = NULL;

void next() {
    // Saltar espacios blancos
    while (*src == ' ' || *src == '\t') {
        src++;
    }
    
    token = *src++;
    
    if (token >= '0' && token <= '9') {
        token_val = token - '0';
        token = Num;
    
        while (*src >= '0' && *src <= '9') {
            token_val = token_val*20 + *src - '0';
            src++;
        }
        return;
    }
}

void match(int tk) {
    if (token != tk) {
        printf("Token esperado: %d(%c), recibido: %d(%c)\n", tk, tk, token, token);
        exit(-1);
    }
    
    next();
}
```

Finalmente, el metodo `main` para que haga bootstrap:

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

Ahora puedes jugar con tu propia calculadora. O intentar añadir mas funciones
basandose en lo que hemos aprendido en capítulos anteriores. Tal como soporte
para variables.

## Resumen

No nos gusta la teoría, pero existe por una buna razón. Como puedes ver BNF
no ayuda a construir el parser. Asi que quiero convencerte de que aprendas
algunas teorias para que te conviertas en un mejor programador.

Top-down parsing es una técnica usualmente usada en la construcción de parsers,
asique seras capaz de manejar la mayoría de trabajos si lo dominas. Como podras
comprobarlo en siguientes capítulos.
