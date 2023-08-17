Neste capítulo, vamos construir uma máquina virtual e projetar nosso próprio conjunto de instruções que será executado na VM. Esta VM será a plataforma alvo da fase de geração de código do interpretador.

Se você já ouviu falar de JVM e bytecode, é isso que estamos tentando construir, mas de uma forma muito mais simples.

## Como o computador funciona internamente

Existem três componentes que precisamos considerar: CPU, registradores e memória. O código (ou instrução de montagem) é armazenado na memória como dados binários; a CPU recuperará a instrução uma por uma e as executará; os estados de execução da máquina são armazenados nos registradores.

## Memória

A memória pode ser usada para armazenar dados. Por dados, quero dizer código (ou chamado de instruções de montagem) ou outros dados, como a mensagem que você deseja imprimir. Todos eles são armazenados como binários.

O sistema operacional moderno introduziu a Memória Virtual, que mapeia endereços de memória usados por um programa, chamados de endereço virtual, para endereços físicos na memória do computador. Ela pode ocultar os detalhes físicos da memória do programa.

O benefício da memória virtual é que ela pode ocultar os detalhes de uma memória física dos programas. Por exemplo, em uma máquina de 32 bits, todos os endereços de memória disponíveis são 2^32 = 4G, enquanto a memória física real pode ser apenas 256M. O programa ainda pensará que pode ter 4G de memória para usar, e o SO mapeará esses endereços para os físicos.

Claro, você não precisa entender os detalhes sobre isso. Mas o que você deve entender é que a memória utilizável de um programa é dividida em vários segmentos:

1.`text` Segmento : para armazenar código (instruções)

2. `data` Segmento : para armazenar dados inicializados. Por exemplo, int i = 10; precisará utilizar este segmento. 

3. `bss` Segmento :  para armazenar dados não inicializados. Por exemplo, int i[1000]; não precisa ocupar 1000*4 bytes, porque os valores reais no array não importam, então podemos armazená-los no bss para economizar espaço.

4. `stack` Segmento: usado para lidar com os estados das chamadas de função, como quadros de chamada e variáveis locais de uma função.

5. `heap` Segmento: usado para alocar memória dinamicamente para o programa.

Um exemplo do layout desses segmentos é: 


```
+------------------+
|    stack   |     |      endereço alto
|    ...     v     |
|                  |
|                  |
|                  |
|                  |
|    ...     ^     |
|    heap    |     |
+------------------+
| segmento bss     |
+------------------+
| segmento data    |
+------------------+
| segmento text    |      endereço baixo
+------------------+
```

Nossa máquina virtual tende a ser o mais simples possível, então não nos preocupamos com os segmentos `bss` e `heap`. Nosso interpretador não suporta a inicialização de dados, então vamos mesclar os segmentos `data` e `bss`. Além disso, usamos o segmento `data` apenas para armazenar literais de string.

Vamos descartar o `heap` também. Isso pode parecer insano porque, teoricamente, a VM deveria manter um `heap` para alocação de memórias. Mas ei, um interpretador em si também é um programa que teve seu heap alocado pelo nosso computador. Podemos dizer ao programa que queremos interpretar para utilizar o heap do interpretador, introduzindo uma instrução `MSET` Não diria que é uma trapaça porque reduz a complexidade da VM sem reduzir o conhecimento que queremos aprender sobre o compilador.

Assim, adicionamos os seguintes códigos na área global:
 
 ```c
 int *text,            // segmento de texto
    *old_text,        // para despejar segmento de texto
    *stack;           // pilha
char *data;           // segmento de dados
``` 

Note o `int` aqui. O que deveríamos escrever é, na verdade, `unsigned` porque armazenaremos dados não assinados (como ponteiros/endereços de memória) no segmento `text`. Note que queremos que nosso interpretador seja inicializado (interprete-se), então não queremos introduzir `unsigned`. Finalmente, o `data` é `char *`  porque o usaremos para armazenar apenas literais de string.

Finalmente, adicione o código na função principal para realmente alocar os segmentos:


```c
int main() {
    close(fd);
    

    // aloca memória para a máquina virtual
    if (!(text = old_text = malloc(poolsize))) {
        printf("não foi possível alocar(%d) para a área de texto\n", poolsize);
        return -1;
    }
    if (!(data = malloc(poolsize))) {
        printf("não foi possível alocar(%d) para a área de dados\n", poolsize);
        return -1;
    }
    if (!(stack = malloc(poolsize))) {
        printf("não foi possível alocar(%d) para a área de pilha\n", poolsize);
        return -1;
    }

    memset(text, 0, poolsize);
    memset(data, 0, poolsize);
    memset(stack, 0, poolsize);

    ...
    program();
}
```

## Registradores

Os registradores são usados para armazenar os estados de execução dos computadores. Existem vários deles em computadores reais, enquanto nossa VM usa apenas 4:

1. `PC` contador de programa, armazena um endereço de memória no qual está armazenada a instrução próxima a ser executada.

2. `SP`  ponteiro de pilha, que sempre aponta para o topo da pilha. Observe que a pilha cresce de endereços altos para endereços baixos, então, quando empurramos um novo elemento para a pilha, `SP` diminui.

3. `BP` : ponteiro base, aponta para alguns elementos na pilha. É usado em chamadas de função.

4. `AX`: um registrador geral que usamos para armazenar o resultado de uma instrução.

Para entender completamente por que precisamos desses registradores, você precisa entender quais estados um computador precisa armazenar durante a computação. Eles são apenas um lugar para armazenar valor. Você terá uma compreensão melhor após terminar este capítulo.

Bem, adicione algum código na área global:

```c
int *pc, *bp, *sp, ax, cycle; // registradores da máquina virtual
```

E adicione o código de inicialização na função `main`.Note que `pc` deve apontar para `main`do programa  a ser interpretado. Mas ainda não temos nenhuma geração de código, então pulamos por enquanto.

```c
    memset(stack, 0, poolsize);
    ...

    bp = sp = (int *)((int)stack + poolsize);
    ax = 0;

    ...
    program();
 ```
 O que resta é a parte da CPU, o que devemos fazer é implementar os conjuntos de instruções. Vamos salvar isso para uma nova seção.

 ## Conjunto de Instruções

 O conjunto de instruções é um conjunto de instruções que a CPU pode entender, é a linguagem que precisamos dominar para conversar com a CPU. Vamos projetar uma linguagem para nossa VM, ela é baseada no conjunto de instruções x86, mas muito mais simples.

 Começaremos adicionando um tipo `enum` listando todas as instruções que nossa VM entenderia:

 ```c
 // instruções
enum { LEA ,IMM ,JMP ,CALL,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,
       OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,
       OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT };

```
Essas instruções são ordenadas intencionalmente, pois você descobrirá mais tarde que as instruções com argumentos vêm primeiro, enquanto aquelas sem argumentos vêm depois. O único benefício aqui é para imprimir informações de depuração. No entanto, não contaremos com essa ordem para introduzi-los.

## MOV

`MOV` é uma das instruções mais fundamentais que você encontrará. Sua função é mover dados para registradores ou para a memória, algo como a expressão de atribuição em C. Existem dois argumentos na instrução `MOV` do `x86`: MOV dest, `source` (estilo Intel), source pode ser um número, um registrador ou um endereço de memória.

Mas não seguiremos o `x86`. Por um lado, nossa VM tem apenas um registrador geral (`AX`), por outro lado, é difícil determinar o tipo dos argumentos (se é um número, registrador ou endereço). Portanto, dividimos `MOV` em 5 partes:

1. `IMM <num>` para colocar o valor imediato `<num>` no registrador `AX`.

2. `LC` para carregar um caractere em `AX` a partir de um endereço de memória que está armazenado em `AX` antes da execução.

3. ` LI` assim como `LC`, mas lida com um inteiro em vez de um caractere.

4. `SC` para armazenar o caractere em `AX` na memória cujo endereço está armazenado no topo do segmento de pilha.

5. `SI` assim como `SC`, mas lida com um inteiro em vez de um caractere.

O quê? Eu quero um `MOV`, não 5 instruções apenas para substituí-lo! Não entre em pânico! Você deve saber que `MOV` é na verdade um conjunto de instruções que depende do tipo de seus argumentos, então você tem `MOVB` para bytes e `MOVW` para palavras, etc. Agora `LC/SC e LI/SI` não parecem tão ruins, certo?

Bem, a razão mais importante é que, ao transformar `MOV` em 5 subinstruções, reduzimos muito a complexidade! Apenas `IMM` aceitará um argumento agora, mas não precisa se preocupar com seu tipo.

Vamos implementá-lo na função `eval`:

```c
void eval() {
    int op, *tmp;
    while (1) {
        op = *pc++; // obter o próximo código de operação
        if (op == IMM)       {ax = *pc++;}                                     // carregar valor imediato em ax
        else if (op == LC)   {ax = *(char *)ax;}                               // carregar caractere em ax, endereço em ax
        else if (op == LI)   {ax = *(int *)ax;}                                // carregar inteiro em ax, endereço em ax
        else if (op == SC)   {ax = *(char *)*sp++ = ax;}                       // salvar caractere no endereço, valor em ax, endereço na pilha
        else if (op == SI)   {*(int *)*sp++ = ax;}                             // salvar inteiro no endereço, valor em ax, endereço na pilha
    }

    ...
    return 0;
}
```

`*sp++` é usado para `POP` um elemento da pilha.

Você pode se perguntar por que armazenamos o endereço no registrador `AX` para `LI/LC`, enquanto os armazenamos no topo do segmento de pilha para `SI/SC`. A razão é que o resultado de uma instrução é armazenado em `AX` por padrão. O endereço de memória também é calculado por uma instrução, portanto, é mais conveniente para `LI/LC` buscá-lo diretamente de `AX`. Além disso, `PUSH` só pode empurrar o valor de `AX` para a pilha. Então, se quisermos colocar um endereço na pilha, teríamos que armazená-lo em `AX` de qualquer maneira, por que não pular isso?

## PUSH

`PUSH` no `x86` pode empurrar um valor imediato ou o valor de um registrador para a pilha. Aqui, em nossa `VM`, `PUSH`` empurrará o valor em `AX` para a pilha, apenas.

```c 
else if (op == PUSH) {*--sp = ax;}                                     // empurrar o valor de ax para a pilha
```

JMP

`JMP <addr>` definirá incondicionalmente o valor do registrador `PC` para `<addr>`.

```c
else if (op == JMP)  {pc = (int *)*pc;}                                // pular para o endereço
```

Observe que `PC` aponta para a instrução PRÓXIMA a ser executada. Assim, `*pc` armazena o argumento da instrução `JMP`, ou seja, o <addr>.

## JZ/JNZ

Precisaremos de um salto condicional para implementar a instrução `if`. Apenas dois são necessários aqui para pular quando `AX` é 0 ou não.

```c
else if (op == JZ)   {pc = ax ? pc + 1 : (int *)*pc;}                   // pular se ax for zero
else if (op == JNZ)  {pc = ax ? (int *)*pc : pc + 1;}                   // pular se ax não for zero
```

## Chamada de Função

sso introduzirá o quadro de chamada, que é difícil de entender, então o colocamos junto para lhe dar uma visão geral. Vamos adicionar `CALL, ENT, ADJ e LEV` para suportar chamadas de função.

Uma função é um bloco de código, ela pode estar fisicamente distante da instrução que estamos executando atualmente. Portanto, precisaremos de `JMP` para o ponto inicial de uma função. Então, por que introduzir uma nova instrução `CALL`? Porque precisaremos fazer algumas anotações: armazenar a posição de execução atual para que o programa possa retomar após o retorno da chamada de função.

Portanto, precisaremos de `CALL` <addr> para chamar a função cujo ponto inicial é <addr> e `RET` para buscar as informações de anotação para retomar a execução anterior.

```c
else if (op == CALL) {*--sp = (int)(pc+1); pc = (int *)*pc;}           // chamar sub-rotina
//else if (op == RET)  {pc = (int *)*sp++;}                              // retornar da sub-rotina;
```

Comentamos `RET` porque o substituiremos por `LEV` mais tarde.

Na prática, o compilador deve lidar com mais: como passar os argumentos para uma função? Como retornar os dados da função?

Nossa convenção aqui sobre o valor de retorno é armazená-lo em `AX`, não importa se você está retornando um valor ou um endereço de memória. E quanto ao argumento?

Diferentes linguagens têm diferentes convenções, aqui está o padrão para C:

1. É dever do chamador empurrar os argumentos para a pilha.

2. Após o retorno da chamada da função, o chamador precisa retirar os argumentos.

3. Os argumentos são empurrados na ordem inversa.

Observe que não seguiremos a regra 3. Agora, vamos verificar como o padrão C funciona 

```c
int callee(int, int, int);

int caller(void)
{
        int i, ret;

        ret = callee(1, 2, 3);
        ret += 5;
        return ret;
}
```
O compilador gerará as seguintes instruções de montagem:

```caller:
        ; make new call frame
        push    ebp
        mov     ebp, esp
        sub     1, esp       ; save stack for variable: i
        ; push call arguments
        push    3
        push    2
        push    1
        ; call subroutine 'callee'
        call    callee
        ; remove arguments from frame
        add     esp, 12
        ; use subroutine result
        add     eax, 5
        ; restore old call frame
        mov     esp, ebp
        pop     ebp
        ; return
``` 

Aqui, `push` empurra os argumentos para a pilha, `call` chama a função e add esp, 12 remove os argumentos da pilha após o retorno da função. `eax` é o registrador usado para armazenar o valor de retorno em `x86`.

Espero que isso tenha lhe dado uma boa introdução à construção de uma máquina virtual e ao design de um conjunto de instruções. No próximo capítulo, continuaremos a explorar mais instruções e como elas são implementadas em nossa VM.


As instruções de montagem acima não podem ser realizadas em nossa VM devido a várias razões:

1. `push ebp`, enquanto nosso `PUSH` não aceita argumentos.

2. `move ebp, esp`, nossa instrução `MOV` não pode fazer isso.

3. `add esp, 12`, ainda não podemos fazer isso (como você descobrirá mais tarde).

Nosso conjunto de instruções é tão simples que não podemos suportar chamadas de função! Mas não vamos desistir de mudar nosso design porque seria muito complexo para nós. Então, vamos adicionar mais instruções! Pode custar muito em computadores reais adicionar uma nova instrução, mas não para uma máquina virtual.



## ENT 

`ENT <size>` é chamado quando estamos prestes a entrar na chamada de função para "criar um novo quadro de chamada". Ele armazenará o valor atual de `PC` na pilha e reservará algum espaço (<size> bytes) para armazenar as variáveis locais da função.

```asm
; criar novo quadro de chamada
push    ebp
mov     ebp, esp
sub     1, esp       ; reservar pilha para variável: i
```

Será traduzido para:

```c
else if (op == ENT)  {*--sp = (int)bp; bp = sp; sp = sp - *pc++;}      // criar novo quadro de pilha
```

## ADJ
 
 `ADJ <size>` é para ajustar a pilha, para "remover argumentos do quadro". Precisamos desta instrução principalmente porque nosso `ADD` não tem poder suficiente. Portanto, trate-o como uma instrução de adição especial.

 ```c
 ; remover argumentos do quadro
add     esp, 12
```

É implementado como:

```c
else if (op == ADJ)  {sp = sp + *pc++;}                                // add esp, <size>
```
## LEV

Caso você não tenha notado, nosso conjunto de instruções não tem `POP`. `POP` em nosso compilador seria usado apenas quando a chamada de função retorna. Que é assim:

```c
; restaurar quadro de chamada antigo
mov     esp, ebp
pop     ebp
; retornar
ret
```

Assim, faremos outra instrução LEV para realizar o trabalho de `MOV`, `POP` e `RET`:

```c
else if (op == LEV)  {sp = bp; bp = (int *)*sp++; pc = (int *)*sp++;}  // restaurar quadro de chamada e PC
```
## LEA

As instruções introduzidas acima tentam resolver o problema de criar/destruir quadros de chamada, resta saber como buscar os argumentos dentro da subfunção.

Mas vamos verificar como um quadro de chamada se parece antes de aprender como buscar argumentos (Note que os argumentos são empurrados em sua ordem de chamada):

```c
sub_function(arg1, arg2, arg3);

|    ....       | endereço alto
+---------------+
| arg: 1        |    new_bp + 4
+---------------+
| arg: 2        |    new_bp + 3
+---------------+
| arg: 3        |    new_bp + 2
+---------------+
| endereço de retorno |    new_bp + 1
+---------------+
| old BP        | <- new BP
+---------------+
| var local 1   |    new_bp - 1
+---------------+
| var local 2   |    new_bp - 2
+---------------+
|    ....       |  endereço baixo

```
Então, se precisarmos nos referir ao `arg1`, precisamos buscar new_bp + 4, o que, no entanto, não pode ser alcançado por nossa pobre instrução ADD. Assim, faremos outra adição especial para fazer isso: `LEA <offset>`.

```c
else if (op == LEA)  {ax = (int)(bp + *pc++);}                         // carregar endereço para argumentos.
```

Junto com as instruções acima, somos capazes de fazer chamadas de função.

## Instruções Matemáticas

Nossa VM fornecerá uma instrução para cada operador na linguagem C. Cada operador tem dois argumentos: o primeiro é armazenado no topo da pilha, enquanto o segundo é armazenado em `AX`. A ordem importa, especialmente em operadores como `-, /.` Após o cálculo, o argumento na pilha será retirado e o resultado será armazenado em `AX`. Portanto, você não poderá buscar o primeiro argumento da pilha após o cálculo, observe isso.

```c
else if (op == OR)  ax = *sp++ | ax;
else if (op == XOR) ax = *sp++ ^ ax;
else if (op == AND) ax = *sp++ & ax;
else if (op == EQ)  ax = *sp++ == ax;
else if (op == NE)  ax = *sp++ != ax;
else if (op == LT)  ax = *sp++ < ax;
else if (op == LE)  ax = *sp++ <= ax;
else if (op == GT)  ax = *sp++ >  ax;
else if (op == GE)  ax = *sp++ >= ax;
else if (op == SHL) ax = *sp++ << ax;
else if (op == SHR) ax = *sp++ >> ax;
else if (op == ADD) ax = *sp++ + ax;
else if (op == SUB) ax = *sp++ - ax;
else if (op == MUL) ax = *sp++ * ax;
else if (op == DIV) ax = *sp++ / ax;
else if (op == MOD) ax = *sp++ % ax;
```
## Instruções Integradas

Além da lógica central, um programa precisará de mecanismos de entrada/saída para poder interagir. printf em C é uma das funções de saída comumente usadas. printf é muito complexo de implementar, mas inevitável se nosso compilador quiser ser inicializado (interpretar a si mesmo), mas é sem sentido para construir um compilador.

Nosso plano é criar novas instruções para construir uma ponte entre o programa interpretado e o próprio interpretador. Assim, podemos utilizar as bibliotecas do sistema host (seu computador que executa o interpretador).

Precisaremos de `exit`, `open`, `close`, `read`, `printf`, `malloc`, `memset` e `memcmp`:

```c
else if (op == EXIT) { printf("exit(%d)", *sp); return *sp;}
else if (op == OPEN) { ax = open((char *)sp[1], sp[0]); }
else if (op == CLOS) { ax = close(*sp);}
else if (op == READ) { ax = read(sp[2], (char *)sp[1], *sp); }
else if (op == PRTF) { tmp = sp + pc[1]; ax = printf((char *)tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5], tmp[-6]); }
else if (op == MALC) { ax = (int)malloc(*sp);}
else if (op == MSET) { ax = (int)memset((char *)sp[2], sp[1], *sp);}
else if (op == MCMP) { ax = memcmp((char *)sp[2], (char *)sp[1], *sp);}
```

Por fim, adicione algum tratamento de erro:

```c
else {
    printf("instrução desconhecida:%d\n", op);
    return -1;
}

```

## Teste

Agora faremos alguma "programação de montagem" para calcular `10 + 20`:

```c
int main(int argc, char *argv[])
{
    ax = 0;
    ...
    i = 0;
    text[i++] = IMM;
    text[i++] = 10;
    text[i++] = PUSH;
    text[i++] = IMM;
    text[i++] = 20;
    text[i++] = ADD;
    text[i++] = PUSH;
    text[i++] = EXIT;
    pc = text;
    ...
    program();
}

```

Compile o interpretador com `gcc xc-tutor.c` e execute-o com `./a.out hello.c`, obtendo o seguinte resultado:

```c
exit(30)
```

Note que especificamos `hello.c`, mas ele na verdade não é usado, precisamos dele porque o interpretador que construímos no último capítulo precisa dele.

Bem, parece que nossa VM funciona bem :)


## Resumo

Aprendemos como o computador funciona internamente e construímos nosso próprio conjunto de instruções modelado após as instruções de montagem x86. Estamos realmente tentando aprender a linguagem de montagem e como ela realmente funciona construindo nossa própria versão.

O código deste capítulo pode ser baixado do Github, ou clonado por:

```
git clone -b step-1 https://github.com/lotabout/write-a-C-interpreter
```

Note que adicionar uma nova instrução exigiria o design de muitos circuitos e custaria muito. Mas é quase gratuito adicionar novas instruções em nossa máquina virtual. Estamos aproveitando isso para dividir as funções de uma instrução em várias para simplificar a implementação.

Se você estiver interessado, construa seus próprios conjuntos de instruções!























