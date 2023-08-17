> A análise léxica é o processo de converter uma sequência de caracteres (como
> em um programa de computador ou página da web) em uma sequência de tokens (strings com
> um "significado" identificado).

Normalmente representamos o token como um par: `(tipo de token, valor do token)`. Por
exemplo, se um arquivo fonte de um programa contém a string: "998", o lexer irá
tratá-lo como token `(Número, 998)`, significando que é um número com valor de `998`.

## Lexer vs Compilador

Vamos primeiro olhar para a estrutura de um compilador:
```
                   +-------+                      +--------+
--codigo fonte -->| lexer | --> fluxo de tokens --> | parser | --> assembly
                   +-------+                      +--------+
```


O Compilador pode ser tratado como um transformador que converte código fonte C em
assembly. Nesse sentido, lexer e parser são transformadores também: Lexer
pega o código fonte C como entrada e produz um fluxo de tokens; O Parser irá consumir o
fluxo de tokens e gerar código assembly.

Então, por que precisamos de um lexer e um parser? Bem, o trabalho do Compilador é difícil! Então nós
recrutamos o lexer para fazer parte do trabalho e o parser para fazer o resto, para que cada
um só precise lidar com uma tarefa simples.

Esse é o valor de um lexer: simplificar o parser convertendo o fluxo
de código fonte em fluxo de tokens.

## Escolha de Implementação

Antes de começarmos, quero que saiba que criar um lexer é tedioso e
propenso a erros. É por isso que gênios já criaram ferramentas automáticas
para fazer o trabalho. `lex/flex` são exemplos que nos permitem descrever as
regras léxicas usando expressões regulares e gerar um lexer para nós.

Também note que não seguiremos o gráfico na seção acima, ou seja, não
converteremos todo o código fonte em fluxo de tokens de uma vez. As razões são:

1. Converter código fonte em fluxo de tokens é um processo com estado. Como uma string é
   interpretada está relacionado com o lugar onde ela aparece.
2. É um desperdício armazenar todos os tokens porque apenas alguns deles serão
   acessados ao mesmo tempo.

Assim, implementaremos uma função: `next()` que retorna um token em uma chamada.

## Tokens Suportados

Adicione a definição na área global:

```c
// tokens e classes (operadores por último e em ordem de precedência)
enum {
  Num = 128, Fun, Sys, Glo, Loc, Id,
  Char, Else, Enum, If, Int, Return, Sizeof, While,
  Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak
};
```

Estes são todos os tokens que nosso lexer pode entender. Nosso lexer interpretará
a string = como token `Assign`; `==` como token `Eq`; != como `Ne`; etc.

Então temos a impressão de que um token conterá um ou mais caracteres.
Essa é a razão pela qual o lexer pode reduzir a complexidade, agora o parser não
precisa olhar vários caracteres para identificar o significado de uma substring.
O trabalho já foi feito.

Claro, os tokens acima estão devidamente ordenados refletindo sua prioridade em
a linguagem de programação C. O operador `*(Mul)`, por exemplo, tem prioridade mais alta
que o operador `+(Add)`. Falaremos sobre isso mais tarde.

Por fim, existem alguns caracteres que não incluímos aqui que são eles mesmos um
token como `]` ou `~`. A razão de não codificá-los como outros são:

1. Esses tokens contêm apenas um único caractere, portanto, são mais fáceis de identificar.
2. Eles não estão envolvidos na batalha de prioridade.

## Esqueleto do Lexer

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

Por que precisamos de `while` aqui sabendo que `next` só analisará um token?
Isso levanta uma questão na construção do compilador (lembre-se de que o lexer é um tipo
de compilador?): Como lidar com erros?

Normalmente temos duas soluções:

1. aponta onde o erro ocorre e sai.
2. aponta onde o erro ocorre, ignora e continua.

Isso explicará a existência de `while`: para pular caracteres desconhecidos no
código fonte. Enquanto isso, é usado para pular espaços em branco que não são a parte real
de um programa. Eles são tratados apenas como separadores.

## Nova Linha

É bastante semelhante ao espaço, pois estamos pulando. A única diferença é que
precisamos aumentar o número da linha quando um caractere de nova linha é encontrado:

```c
// analisa token aqui

...
if (token == '\n') {
    ++line;
}
```

## Macros

Macros em C começam com o caractere `#` como `#include <stdio.h>`. Nosso
compilador não suporta nenhum macro, então vamos pular todos eles:

```c
else if (token == '#') {
    // pula macro, porque não vamos suportá-lo
    while (*src != 0 && *src != '\n') {
        src++;
    }
}
```

## Identificadores e Tabela de Símbolos

Identificador é o nome de uma variável. Mas não nos importamos realmente com os
nomes no lexer, nos importamos com a identidade. Por exemplo: `int a;`
declara uma variável, temos que saber que a declaração `a = 10` que vem
depois refere-se à mesma variável que declaramos anteriormente.

Com base nisso, faremos uma tabela para armazenar todos os nomes que já
encontramos e chamaremos de Tabela de Símbolos. Vamos consultar a tabela quando um novo
nome/identificador é encontrado. Se o nome existir na tabela de símbolos, a
identidade é retornada.

Então, como representar uma identidade?

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
