Já vimos como as definições de variáveis são analisadas em nosso interpretador. Agora é a vez das definições de funções (note que é definição, não declaração, portanto nosso interpretador não suporta recursão entre funções).

## Gramática EBNF

Vamos começar relembrando a gramática EBNF apresentada no último capítulo. Já implementamos `program`, `global_declaration` e `enum_decl`. Lidaremos com parte de variable_decl, function_decl, `parameter_decl` e `body_decl`. O restante será abordado no próximo capítulo.


```
variable_decl ::= type {'*'} id { ',' {'*'} id } ';'

function_decl ::= type {'*'} id '(' parameter_decl ')' '{' body_decl '}'

parameter_decl ::= type {'*'} id {',' type {'*'} id}

body_decl ::= {variable_decl}, {statement}

statement ::= non_empty_statement | empty_statement

non_empty_statement ::= if_statement | while_statement | '{' statement '}'
                     | 'return' expression | expression ';'

if_statement ::= 'if' '(' expression ')' statement ['else' non_empty_statement]

while_statement ::= 'while' '(' expression ')' non_empty_statement
```


## Definição de Função

Lembre-se de que já encontramos funções ao lidar com `global_declaration`:

```c
...
if (token == '(') {
    current_id[Class] = Fun;
    current_id[Value] = (int)(text + 1); // o endereço de memória da função
    function_declaration();
} else {
...
```

O tipo para o identificador atual (ou seja, o nome da função) já havia sido definido corretamente. O trecho de código acima define o tipo (ou seja, `Fun)` e o endereço no `segmento de texto` para a função. Agora entram `parameter_decl` e `body_decl`.

## Parâmetros e Saída em Assembly

Antes de colocar a mão na massa, precisamos entender o código em assembly que será gerado para uma função. Considere o seguinte:


```c
int demo(int param_a, int *param_b) {
    int local_1;
    char local_2;

    ...
}
```

Quando demo é chamada, seu quadro de chamadas (estados da pilha) se parecerá com o seguinte (consulte a VM do capítulo 2):


Neste caso, precisamos entender como os parâmetros e as variáveis locais serão manipulados no código em assembly. Isso é crucial para implementar corretamente a `parameter_decl` e `body_decl` no nosso interpretador.


```
|    ....       | endereço alto
+---------------+
| arg: param_a  |    new_bp + 3
+---------------+
| arg: param_b  |    new_bp + 2
+---------------+
| endereço de retorno |    new_bp + 1
+---------------+
| BP antigo     | <- novo BP
+---------------+
| local_1       |    new_bp - 1
+---------------+
| local_2       |    new_bp - 2
+---------------+
|    ....       | endereço baixo

```

O ponto chave aqui é que, independentemente de se tratar de um parâmetro (por exemplo, `param_a`) ou de uma variável local (por exemplo, `local_1`), todos são armazenados na `pilha`. Portanto, eles são referenciados pelo ponteiro `new_bp` e deslocamentos relativos, enquanto variáveis globais armazenadas no segmento de texto são referidas por endereço direto. Assim, precisamos saber o número de parâmetros e o deslocamento de cada um.


## Esqueleto para Análise de Função

Aqui, você precisa desenvolver a estrutura básica do código que vai analisar a definição da função, incluindo `parameter_decl e body_decl`. Isso vai envolver lidar com a pilha e os endereços de forma apropriada, para garantir que a função seja executada corretamente.
```c
void function_declaration() {
    // tipo nome_func (...) {...}
    //                 | essa parte

    match('(');
    function_parameter();
    match(')');
    match('{');
    function_body();
    //match('}');                  //  ①

    // ②
    // desfaz as declarações de variáveis locais para todas as variáveis locais.
    current_id = symbols;
    while (current_id[Token]) {
        if (current_id[Class] == Loc) {
            current_id[Class] = current_id[BClass];
            current_id[Type]  = current_id[BType];
            current_id[Value] = current_id[BValue];
        }
        current_id = current_id + IdSize;
    }
}
```
Note que supostamente deveríamos consumir o último caractere } em ①. No entanto, não fazemos isso porque `variable_decl` e `function_decl` são analisados juntos (devido ao mesmo prefixo na gramática EBNF) dentro de `global_declaration`. variable_decl termina com ; enquanto `function_decl` termina com }. Se } for consumido, o loop `while` em `global_declaration` não poderá saber que o parsing de function_decl terminou. Portanto, deixamos para `global_declaration` consumi-lo.


O que ② está tentando fazer é desfazer as declarações de variáveis locais para todas as variáveis locais. Como sabemos, as variáveis locais podem ter o mesmo nome que as globais; quando isso acontece, as globais ficam ofuscadas. Portanto, devemos recuperar o status assim que sairmos do corpo da função. As informações sobre variáveis globais são armazenadas nos campos `BXXX`, então iteramos sobre todos os identificadores para recuperá-los.

## function_parameter()

```
parameter_decl ::= type {'*'} id {',' type {'*'} id}
```
É bastante direto, exceto que precisamos lembrar os tipos e posições dos parâmetros.

```C
int index_of_bp; // índice do ponteiro bp na pilha

void function_parameter() {
    int type;
    int params;
    params = 0;
    while (token != ')') {
        // ①

        // int nome, ...
        type = INT;
        if (token == Int) {
            match(Int);
        } else if (token == Char) {
            type = CHAR;
            match(Char);
        }

        // tipo ponteiro
        while (token == Mul) {
            match(Mul);
            type = type + PTR;
        }

        // nome do parâmetro
        if (token != Id) {
            printf("%d: declaração de parâmetro ruim\n", line);
            exit(-1);
        }
        if (current_id[Class] == Loc) {
            printf("%d: declaração duplicada de parâmetro\n", line);
            exit(-1);
        }

        match(Id);

        // ②
        // armazena a variável local
        current_id[BClass] = current_id[Class]; current_id[Class]  = Loc;
        current_id[BType]  = current_id[Type];  current_id[Type]   = type;
        current_id[BValue] = current_id[Value]; current_id[Value]  = params++;  // índice do parâmetro atual

        if (token == ',') {
            match(',');
        }
    }

    // ③
    index_of_bp = params+1;
}
```

Neste exemplo de código C, a função `function_declaration` é responsável por analisar a declaração de uma função, enquanto `function_parameter` trata dos parâmetros da função. O esqueleto oferece uma estrutura básica para manipular declarações de funções em um interpretador C simplificado.

É bastante direto, exceto que precisamos lembrar os tipos e posições dos parâmetros.

```c
int index_of_bp; // índice do ponteiro bp na pilha

void function_parameter() {
    int type;
    int params;
    params = 0;
    while (token != ')') {
        // ①

        // int nome, ...
        type = INT;
        if (token == Int) {
            match(Int);
        } else if (token == Char) {
            type = CHAR;
            match(Char);
        }

        // tipo ponteiro
        while (token == Mul) {
            match(Mul);
            type = type + PTR;
        }

        // nome do parâmetro
        if (token != Id) {
            printf("%d: declaração de parâmetro ruim\n", line);
            exit(-1);
        }
        if (current_id[Class] == Loc) {
            printf("%d: declaração duplicada de parâmetro\n", line);
            exit(-1);
        }

        match(Id);

        // ②
        // armazena a variável local
        current_id[BClass] = current_id[Class]; current_id[Class]  = Loc;
        current_id[BType]  = current_id[Type];  current_id[Type]   = type;
        current_id[BValue] = current_id[Value]; current_id[Value]  = params++;   // índice do parâmetro atual

        if (token == ',') {
            match(',');
        }
    }

    // ③
    index_of_bp = params+1;
}

```
A parte ① é a mesma que vimos em `global_declaration`, que é usada para analisar o tipo do parâmetro.

A parte ② é para fazer backup das informações das variáveis globais que serão ofuscadas por variáveis locais. A posição do parâmetro atual é armazenada no campo `Value`.


A parte ③ é usada para calcular a posição do ponteiro `bp`, que corresponde ao `new_bp` sobre o qual falamos na seção acima.

## function_body()


Diferentemente do C moderno, nosso interpretador requer que todas as definições de variáveis que são usadas na função atual sejam colocadas no início da função atual. Essa regra é, na verdade, a mesma dos antigos compiladores C.


```c
void function_body() {
    // type func_name (...) {...}
    //                   -->|   |<--

    // ... {
    // 1. declarações locais
    // 2. instruções
    // }

    int pos_local; // posição das variáveis locais na pilha.
    int type;
    pos_local = index_of_bp;

    // ①
    while (token == Int || token == Char) {
        // declaração de variável local, assim como as globais.
        basetype = (token == Int) ? INT : CHAR;
        match(token);

        while (token != ';') {
            type = basetype;
            while (token == Mul) {
                match(Mul);
                type = type + PTR;
            }

            if (token != Id) {
                // declaração inválida
                printf("%d: má declaração local\n", line);
                exit(-1);
            }
            if (current_id[Class] == Loc) {
                // identificador já existe
                printf("%d: declaração local duplicada\n", line);
                exit(-1);
            }
            match(Id);

            // armazena a variável local
            current_id[BClass] = current_id[Class]; current_id[Class]  = Loc;
            current_id[BType]  = current_id[Type];  current_id[Type]   = type;
            current_id[BValue] = current_id[Value]; current_id[Value]  = ++pos_local;  // índice do parâmetro atual

            if (token == ',') {
                match(',');
            }
        }
        match(';');
    }

    // ②
    // guarda o tamanho da pilha para variáveis locais
    *++text = ENT;
    *++text = pos_local - index_of_bp;

    // instruções
    while (token != '}') {
        statement();
    }

    // emite código para sair da subfunção
    *++text = LEV;
}

```

Você deve estar familiarizado com ①, já foi repetido várias vezes.

A parte ② está escrevendo código assembly no segmento de texto. No capítulo sobre VM, dissemos que precisamos preservar espaços para variáveis locais na pilha; bem, é isso aí.

## Código

Você pode baixar o código deste capítulo do [Github](https://github.com/lotabout/write-a-C-interpreter/tree/step-4), ou clonar com:

```
git clone -b step-4 https://github.com/lotabout/write-a-C-interpreter
```

O Código Ainda Não Funciona O código ainda não está em execução porque ainda existem algumas funções não implementadas. Você pode se desafiar a completá-las primeiro.


## Resumo

O código deste capítulo não é extenso, sendo a maior parte dele usada para analisar variáveis, e muitas dessas partes são duplicadas. A análise para parâmetros e variáveis locais é quase a mesma, mas as informações armazenadas são diferentes.

É claro que você pode querer revisar o capítulo sobre a Máquina Virtual (capítulo 2) para ter uma melhor compreensão da saída esperada para a função, de modo a entender por que gostaríamos de reunir tais informações. Isso é o que chamamos de "conhecimento de domínio".


Lidaremos com `if` e `while` no próximo capítulo, até lá!


