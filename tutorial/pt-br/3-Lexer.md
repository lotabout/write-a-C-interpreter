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

Vamos precisar de uma pequena explicação aqui:

1. `token`: é o tipo de token de um identificador. Teoricamente, deveria ser
   fixado para o tipo `Id`. Mas isso não é verdade porque adicionaremos palavras-chave (por exemplo,
   `if`, `while`) como tipos especiais de identificador.
2. `hash`: o valor hash do nome do identificador, para acelerar a
   comparação na busca na tabela.
3. `name`: bem, o nome do identificador.
4. `class`: Se o identificador é global, local ou constantes.
5. `type`: tipo do identificador, `int`, `char` ou ponteiro.
6. `value`: o valor da variável à qual o identificador aponta.
7. `BXXX`: uma variável local pode ocultar uma variável global. É usado para armazenar
   as globais se isso acontecer.

Uma tabela de símbolos tradicional conterá apenas o identificador único, enquanto nossa
tabela de símbolos armazena outras informações que só serão acessadas pelo parser,
como `type`.

Infelizmente, nosso compilador não suporta `struct` enquanto estamos tentando ser
auto-suficientes. Portanto, temos que fazer um compromisso na estrutura real de um
identificador:

```
Tabela de símbolos:
----+-----+----+----+----+-----+-----+-----+------+------+----
 .. |token|hash|name|type|class|value|btype|bclass|bvalue| ..
----+-----+----+----+----+-----+-----+-----+------+------+----
    |<---       um único identificador                --->|
```


Isso significa que usamos um único array `int` para armazenar todas as informações do identificador.
Cada ID usará 9 células. O código é o seguinte:

```c
int token_val;                // valor do token atual (principalmente para números)
int *current_id,              // ID analisado atualmente
    *symbols;                 // tabela de símbolos
// campos do identificador
enum {Token, Hash, Name, Type, Class, Value, BType, BClass, BValue, IdSize};
void next() {
        ...
        else if ((token >= 'a' && token <= 'z') || (token >= 'A' && token <= 'Z') || (token == '_')) {
            // analisa identificador
            last_pos = src - 1;
            hash = token;
            while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') || (*src == '_')) {
                hash = hash * 147 + *src;
                src++;
            }
            // procura identificador existente, busca linear
            current_id = symbols;
            while (current_id[Token]) {
                if (current_id[Hash] == hash && !memcmp((char *)current_id[Name], last_pos, src - last_pos)) {
                    // encontrou um, retorna
                    token = current_id[Token];
                    return;
                }
                current_id = current_id + IdSize;
            }
            // armazena novo ID
            current_id[Name] = (int)last_pos;
            current_id[Hash] = hash;
            token = current_id[Token] = Id;
            return;
        }
        ...
}

``` 

Note que a busca na tabela de símbolos é uma busca linear.

## Número

Precisamos suportar decimal, hexadecimal e octal. A lógica é bastante
direta, exceto como obter o valor hexadecimal. Talvez..

```c
token_val = token_val * 16 + (token & 0x0F) + (token >= 'A' ? 9 : 0);
```

Caso você não esteja familiarizado com esse "pequeno truque", o valor hexadecimal de`a`
é `61` em ASCII, enquanto `A` é `41`. Assim, `token & 0x0F` pode obter o dígito mais pequeno
do caractere.

```c
void next() {
        ...



        else if (token >= '0' && token <= '9') {
            // analisa número, três tipos: dec(123) hex(0x123) oct(017)
            token_val = token - '0';
            if (token_val > 0) {
                // dec, começa com [1-9]
                while (*src >= '0' && *src <= '9') {
                    token_val = token_val*10 + *src++ - '0';
                }
            } else {
                // começa com o número 0
                if (*src == 'x' || *src == 'X') {
                    // hex
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

