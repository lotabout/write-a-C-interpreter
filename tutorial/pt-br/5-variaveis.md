Neste capítulo vamos usar EBNF para descrever a gramática do nosso C
intérprete e adicione o suporte de variáveis.

O parser é mais complicado que o lexer, por isso vamos dividi-lo em 3 partes:
variáveis, funções e expressões.

## Gramática EBNF do Interpretador C

Neste capítulo, discutimos a Gramática EBNF (Extended Backus–Naur Form) do nosso interpretador de C. Se você está familiarizado com expressões regulares, deverá se sentir à vontade. Pessoalmente, acho que é mais poderosa e direta do que a BNF. Aqui está a gramática EBNF do nosso interpretador C, sinta-se à vontade para pular se achar que é muito difícil de entender.

```
programa ::= {declaracao_global}+
declaracao_global ::= decl_enum | decl_variavel | decl_funcao

decl_enum ::= 'enum' [id] '{' id ['=' 'num'] {',' id ['=' 'num'] '}'

decl_variavel ::= tipo {'*'} id { ',' {'*'} id } ';'

decl_funcao ::= tipo {'*'} id '(' decl_parametro ')' '{' decl_corpo '}'

decl_parametro ::= tipo {'*'} id {',' tipo {'*'} id}

decl_corpo ::= {decl_variavel}, {instrucao}

instrucao ::= instrucao_nao_vazia | instrucao_vazia

instrucao_nao_vazia ::= instrucao_se | instrucao_enquanto | '{' instrucao '}'
                     | 'return' expressao | expressao ';'

instrucao_se ::= 'if' '(' expressao ')' instrucao ['else' instrucao_nao_vazia]

instrucao_enquanto ::= 'while' '(' expressao ')' instrucao_nao_vazia
```

Deixaremos `expressao` para capítulos posteriores. Nossa gramática não suportará a declaração de funções, o que significa que chamadas recursivas entre funções não são suportadas. E como estamos começando, isso significa que nosso código de implementação não pode usar recursões cruzadas entre funções. (Desculpe pelo capítulo inteiro sobre parsers recursivos top-down.)

Neste capítulo, implementaremos as `decl_enum` e `decl_variavel`.

## programa()

Já definimos a função `programa`, vamos transformá-la em:

```c

void programa() {
    // obter próximo token
    proximo();
    while (token > 0) {
        declaracao_global();
    }
}
```

Eu sei que ainda não definimos `declaracao_global`, às vezes precisamos de um pouco de pensamento otimista de que talvez alguém (digamos, o Bob) a implementará para você. Assim, você pode se concentrar na visão geral a princípio, em vez de mergulhar em todos os detalhes. Essa é a essência do pensamento top-down.

## função declaracao_global()

Agora é nosso dever (não do Bob) implementar a `declaracao_global``. Ela tentará analisar definições de variáveis, definições de tipos (apenas enum é suportado) e definições de funções:

```c
int basetype;    // the type of a declaration, make it global for convenience
int expr_type;   // the type of an expression

void global_declaration() {
    // global_declaration ::= enum_decl | variable_decl | function_decl
    //
    // enum_decl ::= 'enum' [id] '{' id ['=' 'num'] {',' id ['=' 'num'} '}'
    //
    // variable_decl ::= type {'*'} id { ',' {'*'} id } ';'
    //
    // function_decl ::= type {'*'} id '(' parameter_decl ')' '{' body_decl '}'


    int type; // tmp, actual type for variable
    int i; // tmp

    basetype = INT;

    // parse enum, this should be treated alone.
    if (token == Enum) {
        // enum [id] { a = 10, b = 20, ... }
        match(Enum);
        if (token != '{') {
            match(Id); // skip the [id] part
        }
        if (token == '{') {
            // parse the assign part
            match('{');
            enum_declaration();
            match('}');
        }

        match(';');
        return;
    }

    // parse type information
    if (token == Int) {
        match(Int);
    }
    else if (token == Char) {
        match(Char);
        basetype = CHAR;
    }

    // parse the comma seperated variable declaration.
    while (token != ';' && token != '}') {
        type = basetype;
        // parse pointer type, note that there may exist `int ****x;`
        while (token == Mul) {
            match(Mul);
            type = type + PTR;
        }

        if (token != Id) {
            // invalid declaration
            printf("%d: bad global declaration\n", line);
            exit(-1);
        }
        if (current_id[Class]) {
            // identifier exists
            printf("%d: duplicate global declaration\n", line);
            exit(-1);
        }
        match(Id);
        current_id[Type] = type;

        if (token == '(') {
            current_id[Class] = Fun;
            current_id[Value] = (int)(text + 1); // the memory address of function
            function_declaration();
        } else {
            // variable declaration
            current_id[Class] = Glo; // global variable
            current_id[Value] = (int)data; // assign memory address
            data = data + sizeof(int);
        }

        if (token == ',') {
            match(',');
        }
    }
    next();
}
```

Bem, isso é mais do que duas telas de código! Acredito que isso seja uma tradução direta da gramática. No entanto, para ajudar na compreensão, vou explicar alguns pontos:

`Token Antecipado (Lookahead Token)`: A instrução `if (token == xxx)` é usada para observar o próximo token e decidir qual regra de produção usar. Por exemplo, se encontrarmos o token `enum`, sabemos que estamos tentando analisar uma enumeração. Mas se um tipo for analisado, como `int identificador`, ainda não podemos determinar se `identificador` é uma variável ou uma função. Portanto, o analisador deve continuar a olhar adiante para o próximo token. Se encontrarmos (, agora temos certeza de que identificador é uma função; caso contrário, é uma variável.

`Tipo de Variável`: Nosso interpretador de C suporta ponteiros, o que significa que também suporta ponteiros que apontam para outros ponteiros, como `int **data;`. Como representá-los no código? Já definimos os tipos que oferecemos suporte:

```c
// tipos de variável/função
enum { CHAR, INT, PTR };
```

Então usaremos um `int` para armazenar o tipo. Ele começa com um tipo base: `CHAR` ou `INT`. Quando o tipo é um ponteiro que aponta para um tipo base como `int *data;`, adicionamos `PTR` a ele: `tipo = tipo + PTR;`. O mesmo se aplica ao ponteiro de ponteiro; adicionamos outro `PTR` ao tipo, etc.

## enum_declaration

A lógica principal tenta analisar as variáveis separadas por ','. Você precisa prestar atenção à representação das enumerações.

Armazenaremos uma enumeração como uma variável global. No entanto, seu tipo é definido como `Num` em vez de `Glo` para torná-la uma constante em vez de uma variável global normal. As informações de tipo serão usadas posteriormente na análise de `expressions`.

```c
void enum_declaration() {
    // analisar enum [id] { a = 1, b = 3, ...}
    int i;
    i = 0;
    while (token != '}') {
        if (token != Id) {
            printf("%d: identificador de enumeração inválido %d\n", line, token);
            exit(-1);
        }
        next();
        if (token == Assign) {
            // como {a=10}
            next();
            if (token != Num) {
                printf("%d: inicializador de enumeração inválido\n", line);
                exit(-1);
            }
            i = token_val;
            next();
        }

        current_id[Class] = Num;
        current_id[Type] = INT;
        current_id[Value] = i++;

        if (token == ',') {
            next();
        }
    }
}
```

## Miscelânea

Claro, `function_declaration` será introduzido no próximo capítulo. `match` aparece com frequência. É uma função auxiliar que consome o token atual e busca o próximo:

```c
void match(int tk) {
    if (token == tk) {
        next();
    } else {
        printf("%d: token esperado: %d\n", line, tk);
        exit(-1);
    }
}
```

## Código

Você pode baixar o código deste capítulo no [Github](https://github.com/lotabout/write-a-C-interpreter/tree/step-3), ou clonar com:

```c
git clone -b step-3 https://github.com/lotabout/write-a-C-interpreter
```

O código não vai rodar porque ainda existem algumas funções não implementadas. Você pode se desafiar a completá-las primeiro.

## Resumo

EBNF pode ser difícil de entender devido à sua sintaxe (talvez). Mas deve ser fácil seguir este capítulo uma vez que você consiga ler a sintaxe. O que fazemos é traduzir EBNF diretamente para o código C. Portanto, a análise não é, de forma alguma, emocionante, mas você deve prestar atenção à representação de cada conceito.

Falaremos sobre a definição de função no próximo capítulo. Até lá!

