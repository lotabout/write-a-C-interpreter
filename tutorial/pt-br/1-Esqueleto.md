Neste capítulo, teremos uma visão geral da estrutura do compilador.

Antes de começarmos, gostaria de reforçar que é um interpretador que queremos construir. Isso significa que podemos executar um arquivo fonte C como um script. Ele é escolhido principalmente por duas razões:

* O interpretador difere do compilador apenas na fase de geração de código, portanto, ainda aprenderemos todas as técnicas essenciais de construção de um compilador (como análise lexical e análise sintática).

* Construiremos nossa própria máquina virtual e instruções de montagem, o que nos ajudará a entender como os computadores funcionam.

## Três Fases

Dado um arquivo fonte, normalmente o compilador realizará três fases de processamento:

* Análise Lexical: converte strings de origem em fluxo de tokens internos.

* Análise Sintática: consome o fluxo de tokens e constrói a árvore sintática.

* Geração de Código: percorre a árvore sintática e gera código para a plataforma alvo.

A construção de compiladores tornou-se tão madura que as partes 1 e 2 podem ser feitas por ferramentas automáticas. Por exemplo, flex pode ser usado para análise lexical, e bison para análise sintática. Eles são poderosos, mas fazem milhares de coisas nos bastidores. Para entender completamente como construir um compilador, vamos construí-los todos do zero.

Assim, construiremos nosso interpretador nas seguintes etapas:

* Construir nossa própria máquina virtual e conjunto de instruções. Esta é a plataforma alvo que será usada em nossa fase de geração de código.

* Construir nosso próprio analisador léxico para o compilador C.

* Escrever um analisador sintático de descida recursiva por conta própria.

## Esqueleto do nosso compilador

Modelando a partir do c4, nosso compilador inclui 4 funções principais:

* next() para análise lexical; obter o próximo token; ignorará espaços, tabs, etc.

* program() entrada principal para o analisador sintático.

* expression(level): analisar expressão; o nível será explicado em capítulos posteriores.

* eval(): a entrada para a máquina virtual; usado para interpretar instruções alvo.

Por que expression existe quando temos program para análise sintática? Isso ocorre porque o analisador para expressões é relativamente independente e complexo, então o colocamos em um único módulo (função).

O código é o seguinte: 

```c
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#define int long long // trabalhar com alvo de 64 bits

int token;            // token atual
char *src, *old_src;  // ponteiro para a string de código fonte;
int poolsize;         // tamanho padrão de texto/dados/pilha
int line;             // número da linha

void next() {
    token = *src++;
    return;
}

void expression(int level) {
    // não faz nada
}

void program() {
    next();                  // obter o próximo token
    while (token > 0) {
        printf("token é: %c\n", token);
        next();
    }
}

int eval() { // ainda não faz nada
    return 0;
}

int main(int argc, char **argv)
{
    int i, fd;

    argc--;
    argv++;

    poolsize = 256 * 1024; // tamanho arbitrário
    line = 1;

    if ((fd = open(*argv, 0)) < 0) {
        printf("não foi possível abrir(%s)\n", *argv);
        return -1;
    }

    if (!(src = old_src = malloc(poolsize))) {
        printf("não foi possível alocar(%d) para a área de origem\n", poolsize);
        return -1;
    }

    // ler o arquivo fonte
    if ((i = read(fd, src, poolsize-1)) <= 0) {
        printf("read() retornou %d\n", i);
        return -1;
    }

    src[i] = 0; // adicionar caractere EOF
    close(fd);

    program();
    return eval();
}
```

É bastante código para o primeiro capítulo do artigo. No entanto, é simples o suficiente. O código tenta ler um arquivo fonte, caractere por caractere e imprimi-los.

Atualmente, o analisador léxico next() não faz nada além de retornar os caracteres como estão no arquivo fonte. O analisador sintático program() também não cuida de seu trabalho, nenhuma árvore sintática é gerada, nenhum código alvo é gerado.

O importante aqui é entender o significado dessas funções e como elas estão conectadas, pois são o esqueleto do nosso interpretador. Preencheremos eles passo a passo nos próximos capítulos.

## Código

O código deste capítulo pode ser baixado do Github, ou clonado por:

```shell
git clone -b step-0 https://github.com/lotabout/write-a-C-interpreter
```

Note que eu posso corrigir bugs mais tarde, e se houver alguma inconsistência entre o artigo e os ramos de código, siga o artigo. Só atualizarei o código no branch principal.

## Resumo

Após digitar um pouco, temos o compilador mais simples: um compilador que não faz nada. No próximo capítulo, implementaremos a função eval, ou seja, nossa própria máquina virtual. Até lá.


Espero que isso ajude! Se você tiver mais perguntas ou precisar de mais traduções, estou aqui para ajudar.