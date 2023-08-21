Neste capítulo, construiremos uma calculadora simples usando a técnica de análise top-down (análise descendente). Esta é a preparação antes de começarmos a implementar o analisador.

Vou introduzir um pequeno conjunto de teorias, mas não garanto que estejam absolutamente corretas. Por favor, consulte seu livro didático se tiver alguma dúvida.

## Análise top-down

Tradicionalmente, temos análise top-down e análise bottom-up. O método top-down começa com um não-terminal e verifica recursivamente o código-fonte para substituir os não-terminais por suas alternativas até que nenhum não-terminal reste.

Você viu que usei o método top-down para explicar "top-down" porque você precisa saber o que é um "não-terminal" para entender o parágrafo acima. Mas ainda não te disse o que é isso. Explicaremos na próxima seção. Por agora, considere que "top-down" está tentando decompor um grande objeto em pequenas partes.

Por outro lado, a análise "bottom-up" tenta combinar pequenos objetos em um grande. É frequentemente usado em ferramentas de automação que geram analisadores.

## Terminal e Não-terminal

São termos usados em [BNF](https://pt.wikipedia.org/wiki/Forma_de_Backus-Naur) (Forma de Backus–Naur) que é uma linguagem usada para descrever gramáticas. Uma simples calculadora aritmética elementar em BNF seria:

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

O item delimitado por `<>` é chamado de `Não-terminal`. Eles receberam este nome porque podemos substituí-los pelos itens à direita de `::=`. `|` significa alternativa, isso significa que você pode substituir `<termo>` por qualquer um de `<termo> * <fator>`, `<termo> / <fator>` ou `<fator>`. Aqueles que não aparecem à esquerda de `::=` são chamados de `Terminal`, como `+`, `(`, `Num`, etc. Eles geralmente correspondem aos tokens que obtemos do analisador léxico.

## Exemplo de análise top-down para uma calculadora simples

A árvore de análise é a estrutura interna que obtemos após o analisador consumir todos os tokens e concluir toda a análise. Vamos pegar `3 * (4 + 2)` como exemplo para mostrar as conexões entre gramática BNF, árvore de análise e análise top-down.

A análise top-down começa a partir de um não-terminal inicial, que é `<termo>` em nosso exemplo. Você pode especificá-lo na prática, mas por padrão é o primeiro não-terminal que encontramos.


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


Você pode ver que, a cada etapa, substituímos um não-terminal por uma de suas alternativas (top-down) até que todos os subitens sejam substituídos por terminais (bottom). Alguns não-terminais são usados recursivamente, como `<expr>`.

## Vantagens da análise top-down

Como você pode ver no exemplo acima, a etapa de análise é semelhante à gramática BNF. O que significa que é fácil converter a gramática em código real, convertendo uma regra de produção (`<...> ::= ...`) em uma função com o mesmo nome.

Uma questão surge aqui: como você sabe qual alternativa aplicar? Por que escolher `<expr> ::= <termo> * <fator>` em vez de `<expr> ::= <termo> / <fator>`? Isso mesmo, nós usamos `lookahead`! Nós espiamos o próximo token e é `*`, então é o primeiro a ser aplicado.

No entanto, a análise top-down requer que a gramática não tenha recursão à esquerda.

## Recursão à esquerda

Suponha que tenhamos uma gramática assim:


```
<expr> ::= <expr> + Num
```
Podemos traduzi-la para a função:


```
int expr() {
    expr();
    num();
}
```


Como você pode ver, a função `expr` nunca sairá! Na gramática, o não-terminal `<expr>` é usado recursivamente e aparece imediatamente após `::=`, o que causa recursão à esquerda.

Felizmente, a maioria das gramáticas com recursão à esquerda (talvez todas? Não me lembro) pode ser devidamente transformada em equivalentes não-recursivas à esquerda. Nossa gramática para a calculadora pode ser convertida para:

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


Você deve consultar seu livro didático para obter mais informações.

## Implementação

O código a seguir é diretamente convertido da gramática. Observe como é direto:


```c
int expr();

int factor() {
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
        return term_tail(value);
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

Implementar um analisador sintático "top-down" é simples com a ajuda da gramática BNF.
Agora, adicionamos o código para o lexer:


```c
#include <stdio.h>
#include <stdlib.h>

enum {Num};
int token;
int token_val;
char *line = NULL;
char *src = NULL;

void next() {
    // pula espaços em branco
    while (*src == ' ' || *src == '\t') {
        src ++;
    }

    token = *src++;

    if (token >= '0' && token <= '9' ) {
        token_val = token - '0';
        token = Num;

        while (*src >= '0' && *src <= '9') {
            token_val = token_val*10 + *src - '0';
            src ++;
        }
        return;
    }
}

void match(int tk) {
    if (token != tk) {
        printf("token esperado: %d(%c), recebido: %d(%c)\n", tk, tk, token, token);
        exit(-1);
    }

    next();
}
```
Finalmente, o método main para inicialização:

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

Você pode brincar com sua própria calculadora agora. Ou tente adicionar mais funções baseado no que aprendemos no capítulo anterior. Como suporte a variáveis, para que um usuário possa definir variáveis para armazenar valores.

## Resumo

Não gostamos de teoria, mas ela existe por um bom motivo, como você pode ver que a BNF pode nos ajudar a construir o analisador sintático. Então, quero convencê-lo a aprender algumas teorias, isso ajudará você a se tornar um programador melhor.

A técnica de análise sintática "top-down" é frequentemente usada na criação manual de analisadores, então você é capaz de lidar com a maioria dos trabalhos se dominá-la! Como você verá nos próximos capítulos.