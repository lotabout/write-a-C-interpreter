En este capítulo, vamos a contruir una máquina virtual y diseñar nuestro
propio set de instrucciones que corra en la VM. Este VM sera la plataforma
objetivo de nuestro generador de código.

## Como funciona el ordenador internamente

Hay tres componentes que nos importan: CPU, registradores y RAM. Código
(o instrucciones de ensamblador) son almacenados en la memoria en binario;
La CPU leera las instrucciones uno por uno y los ejecutara; los estados
de la máquina don almacenados en los registros.

### Memoria

La memoria puede ser utilizada para almacenar información. Con información
quiero decir código (o instrucciones de ensamblador) o otra infomación tal
como los mensajes que quieres escribir. Todos estos son almacenados en binario.

Los sistemas operativos modernos introdujeron *VRAM* el cual organiza las
direcciones de memoria utilizados por un programa llamados *virtual adress*
(nota del traductor: no estoy seguro si tiene una traducción al español)
en direcciones físicas en la memoria. Puede esconder los detalles físicos de la
memoria al programa.

El beneficio de la VRAM es que puede esconder los detalles físicos de la
memoria a un programa. Por ejemplo, en una máquina de 32 bits las direcciones
disponibles son `2^32 = 4G` mientras que la memoria física puede ser de
`256M`. El programa aun pensara que tiene disponible `4G`, el OS los
relacionara a direcciones físicas.

Por supuesto, no necesitas entender los detalles. Pero deberias entender que
la memoria uitl de un programa es separado en varios segmentos:

1. `text`: para guardar coigo (instrucciones).
2. `data`: para guardar data inicializada. Por ejemplo `int i = 10;` necesitara
   este segmento.
3. `bss`: para guardar data sin inicializar. Por ejemplo `int i[1000];` no
   necesita ocupar `1000*4` bytes, porque los valores actuales en el array
   no importan, de esta manera podemos almacenarlos en `bss` para ahorrar
   espacio.
4. `stack`: usado para manejar los estados de las llamadas a funciones,
   tal como los marcos de llamada y variables locales.
5. `heap`: usado para allocar memoria dinámicamente para el programa.

Un ejemplo de la organización de estos segmentos es:

```
+------------------+
|    stack   |     |       dirección alta
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
| segmento text    |       dirección baja
+------------------+
```

Nuestra máquina virtual tiende a ser lo más simple posible, de esta manera
no nos importa el `bss` y `heap`. Nuestro interpretador no permite la
inicialización de data, de esta manera vamos a mezclar `data` y `bss`.
Ademas, solo utilizamos `data` para guardar literales de texto.

Nos olvidaremos de `heap` también. Esto suena absurdo porque teóricamente 
la VM debería mantener una `heap` para todas la allocaciones de memoria.
Pero nuestra máquina virtual ya es un programa con su heap. Podemos decirle
al programa que queremos utilizar el heap del programa utilizando la
instrucción `MSET`. No dibre que es hacer trampas porque reduce la complejidad
de la VM sin reducir el conocimiento que necesitamos.

De esta manera añadimos el proximo código en la area global:

```c
int *text,      // segmento text
    *old_text,  // para tirar segmentos de text
	*stack;     // stack
char *data;     // segmento data
```

Notese el `int` aquí. Lo que deberiamos escirbir es `unsigned` porque
guardaremos data positiva como punteros en el segmento `text`. Notese que
queremos que nuestro interpretador pueda interpretarse a si mismo, de esta
manera no queremos introducir `unsigned`. Finalmente `data` es `char*` porque
lo usaremos solo para guardar literales de texto.

Finalmente, añade el código en la función main para allocar los segmentos:

```c
int main() {
    close(fd);
	...
	
	// alloca memoria para la VM
	old_text = malloc(poolsize);
	text = old_text;
	if (!text) {
	    printf("No se pudo allocar(%d) para la area text\n", poolsize);
		return 1;
    }
	data = malloc(poolsize);
	if (!data) {
	    printf("No se pudo allocar(%d) para la area data\n", poolsize);
		return 1;
	}
	stack = malloc(poolsize);
	if (!stack) {
	    printf("No se pudo allocar(%d) para la area data\n", poolsize);
		return 1;
	}
	
	memset(text, 0, poolsize);
	memset(data, 0, poolsize);
	memset(data, 0, poolsize);
	
	...
	program();
}
```

### Registradores

Los registradores se utilizan para guardar los estados de los ordenadores.
Hay varios en los ordenadores reales, nuestra VM tendra 4:

1. `PC`: contadores de programa, guarda un dirección de memoria en el cual
   guarda la instrucción **next** para ejecutarla.
2. `SP`: punteros de stack, el cual siempre apunta al *top* del stack.
   Notese que el stack va de direccion alta a baja, asique cuando un nuevo
   elemento es añadido `SP` se reduce.
3. `BP`: puntero base, apunta a algunos elementos en el stack. Es usado en
   llamadas de función.
4. `AX`: un registro general que usaremos para guardar los resulatdos de una
   instrucción.
   
Para entender completamente porque necesitamos estos registradores, necesitas
entender que stados necesitara un ordenador guardar durente computación. Son
solamente un lugar para guardar valores. Tendras un mayor entedimiento despues
de terminar este capítulo.

Bueno, añade algo de códgo en la area global:

```c
int *pc, *bp, *sp, ac, cycle // registros de la máquina virtual
```

Y añade la inicialización en la función `main`. Notese que `pc` deberia
apuntar a la función `main` del programa a ser ejecutado. Pero no aun no
tenemos generación de código, asique no lo saltamos.

```c
    memset(stack, 0, poolsize);
	...
	
	sp = (int*)((int)stack + poolsize);
	bp = sp;
	ax = 0;
	
	...
	program();
```

Lo que queda es la parte de la CPU, lo que deberiamos hacer es implementar el
set de instrucciones. Dejemoslo para otra sección.

## Set de instrucciones

Un set de instrucciones son instrucciones que la CPU puede entender, es el
lenguaje que necesitamos dominar para hablarle al procesador. Vamos a diseñar
un lenguaje para nuestra máquina virtual, esta basado en el set de
instrucciones x86, pero más simple.

Empezaremos añadiendo un `enum` con todas las instrucciones que nuestra VM
entendera:

```c
// instrucciones
enum {
    LEA, IMM, JMP, CALL, JZ, JNZ, ENT, ADJ, LEV,
	LI, LC, SI, SC, PUSH, OR, XOR, AND, EQ, NE, LT,
	GT, LE, GE, SHL, SHR, ADD, SUB, MIL, DIV, MOD,
	OPEN, READ, CLOS, PTRF, MALC, MSET, MCMP, EXIT
};
```

Estas instrucciones estan ordenadas intencionalmente, decubriras que las
instrucciones con argumentos vienen primero y las sin argumento depues. El
único beneficio que viene en imprimir información de debug. Sin embardo no
dependeremos de esto para introducirlos.

### MOV

`MOV` es una de la más fundamentales instrucciones que encontraras. Su trabajo
es mover información en registros o memoria, como la expression de asignación
en C. Hay dos argumentos en el `MOV` de `x86`: `MOV dest, source` (estilo
Intel), `source` puede ser un numero, registro, o dirección de memoria.

Pero no vamos a seguir `x86`. Por un lado, nuestro VM solo tiene un regitro
general (`AX`), por otro lado es difícil determinar los tipos de argumentos.
De esta manera vamos a separar `MOV` en 5 partes:

1. `IMM <num>` para poner un `<num>` en el registro `AX`.
2. `LC` para cargar un caracter en `AX` de una dirección de memoria guardado
   en `AX` antes de la ejecución.
3. `LI` justo como `LC` pero con enteros en vez de carácteres.
4. `SC` para guardar el carácter en `AX` en la memoria guardado en la cima del
   stack.
5. `SI` justo como `SC` pero con enteros en vez de carácteres.

¿Qué? Quiero un `MOV`, ¡No 5 instrucciones solamente para reemplazarlo! ¡ No
entres en pánico! Deberias de saber que `MOV` es actualmente un set de
instucciones que dependen en el `tipo` de sus argumentos, asique tienes `MOVB`
para bytes y `MOCW` para words, etc. Ahora `LC/SC` y `LI/SI` no parecen tan
malos, ¿Verdad?

Bueno, al convertir `MOV` en 5 sub instrucciones reducimos la complejidad por
mucho. Solo `IMM` aceptara un argumento y por ahora no necesitamos
preocuparnos por su tipo.

Implementemos la función `eval`:

```c
void eval() {
	int op, *tmp;
	while (1) {
	    op = *pc++; // consigue el código de la siguiente operación
		if (op == IMM) {ax = *pc++;} // carga el valor inmediato en ax
		else if (op == LC) {ax = *(char *)ax;} // carga un carácter en ax, dirección en ax
		else if (op == LI) {ax = *(int *)ax;} // carga un entero en ax, direccion en ax
		else if (op == SC) {ax = *(char *)*sp++ = ax;} // guarda un carácter en dirección, valor en ax, dirección en stack
		else if (op == SI) {*(int *)*sp++ = ax;} // guarda un entero en dirección, valor en ax, derección en stack
	}
	
	...
	return 0;
}
```

`*sp++` es usado para `POP` un elemento del stack.

Puede que te preguntes porque guardamos la dirección para `LI/LC` en `AX`
mientras que los guardamos en el stack para `SI/SC`. La razón es que el
resultado de una instrucción es guardado en `AX` por defecto. La memoria
también es calculada por una instrucción, de esta manera es más conveniente para `LI/LC` sacarlo directamente desde `AX`. También `PUSH` solo puede meter
valores de `AX` en el stack. Asique si queremos poner una dirección en el
stack tendremos que guardarlo en `AX`, como sea, ¿Porque no nos lo saltamos?

### PUSH

`PUSH` en `x86` puede empujar un valor inmediato o el valor de un registro
en el stack. En nuestra VM, `PUSH` solamente empujara el valor en `AX` en el
stack.

```c
else if (op == PUSH) {*--sp = ax;}       // empuja el valor de ac en el stack
```

### JMP

`JMP <addr>` incondicionalmente pondra el valor de `PC` a  `<addr>`.

```c
else if (op == JPM) {pc = (int *)*pc;}    // saltar a la dirección
```

Notese que `PC` apunta a la instrucción **NEXT** a ejecutar. De esta manera
`*pc` almancena el argumento de `JMP`, esto es: `<addr>`.

### JZ/JNZ

Necesitaremos un salto condicional para implementar el `if`. Solo dos son
necesarios aquí, uno para saltar cuando `AX` sea `0` o no.

```c
else if (op == JZ) {pc = ax ? pc + 1 : (int *)*pc;} // saltar si ax es cero
else if (op == JNZ) {pc = ax ? (int *)*pc : pc + 1;} // saltar si ax no es cero
```

### Llamada de funciones

Voy a introducir el frame de llamadas el cual es difícil de entender, asique
voy a implementarlo para darte una vista de pajaro. Añadiremos `CALL`, `ENT`,
`ADJ` y `LEV` para poder ejecutar llamadas a funciones.

Una función es un bloque de código, puede que este bastante alejado de las
instrucciones que ahora tenemos. Asique vamos a necesitar hacer un `JMP` al
punto de inicio de una función. Entonces, ¿Para que introducir una nueva
instrucción `CALL`? Porque necesitaremos hacer un poco de libreros: almacenar
la actual ejecución para que el programa pueda resumirse despues de que la
función termine y devuelva.

Añadiremos `CALL <addr>` para llamar a la función cuyo punto de inicio es
`<addr>` y `RET` para tomar la infomación guardada para resumir la ejecución.

```c
else if (op == CALL) {*--sp = (int)(pc+1); pc = (int *)*pc;}  // llamar a la subrutina
//else if (op == RET) {oc = (int *)*sp++;}                      // volver de la subrutina
```

Hemos dejado `RET` comentado porque lo vamos a reemplazar con `LEV` en el
futuro.

En la práctica, el compilador deberia encargarse de más cosas: ¿Como pasamos
argumentos a una función? ¿Como devolver la información desde una función?

Nosotros vamos a devolver valores desde una función poniendolas es `AX` da
igual que estes devolviendo. ¿Pero como lo haremos para los argumentos?

Diferentes lenguajes tienen diferentes convenciones, aquí tienes el standard
para C:

1. Es la responsabilidad de el "caller" empujar los argumentos en el stack.
2. Despues de que la función vuelva, el "caller" tiene que sacar los
   argumentos.
3. Los argumentos son empujados en el orden inverso.

Notese que no vamos a seguir la regla 3. Ahora, vamos a hablar sobre como el
standard de C funciona (de [Wikipedia](https://en.wikipedia.org/wiki/X86_calling_conventions)):

```c
int callee(int, int, int);

int caller(void) {
    int i, ret;
	
	ret = callee(1, 2, 3);
	ret += 5;
	return ret;
}
```

El compilador generara las siguientes instrucciones de ensamblador:

```
caller:
	; crea nuevo marco de llamada
	push   ebp
	mov    ebp, esp
	sub    1, esp    ; guarda stack para variable i
	; empuja los argumentos
	push   3
	push   2
	push   1
	; llama a la subrutina 'callee'
	call   callee
	; saca los argumentos del marco
	add    esp, 13
	; utiliza el resultado de la subrutina
	add    eax, 5
	; recupera el marco anterior
	mov    esp, ebp
	pop    ebp
	; return
	ret
```

Las instrucciones de encima no pueden ser conseguidos con nuestra VM debido a
varias razones:

1. `push ebp` cuando nuestro `PUSH` nisiquiera admite argumentos.
2. `move ebp, esp`, nuestro `MOV` no puede hacer esto.
3. `add esp, 12`, bueno, aún no podemos hacer esto (como descubriras despues).

Nuestras instrucciones son demasiado simples y debido a esto no podemos
emplear funciones. Pero no nos rendimos ante la necesidad de cambiar nuestro
diseño debido a la complejidad añadida. Puede ser muy costoso añadir nuevas
instrucciones en ordenadores reales, pero no para las maquinas virtuales.

### ENT

`ENT <size>` es llamado cuando vamos a entrar en una función para crear el
nuevo marco. Esto almacenará el valor de `PC` en el stack, y guardad algo
de espacio (`<size>` bytes) para guardar las variables locales de la función.

```
; crear nuevo marco de llamada
push    ebp
mov     ebp, esp
sub     1, esp       ; guardar stack para variable: i
```

sera traducido:

```c
else if (op == ENT) {*--sp = (int)bp; bp = sp; sp = sp - *pc++;}  // crear nuevo marco de stack
```

### ADJ

`ADJ <size>`  es para ajustar el stack, para "eliminar argumentos del marco".
Necesitamos esta instrucción principalmente porque nuestro `ADD` no tiene
suficiente poder. Asique tratalo como una función especial.

```
; elimina argumentos del marco
add    esp, 12
```

Es implementado como:

```c
else if (op == ADJ) so = sp + *pc++;}       // add esp, <size>
```

### LEV

En caso de que no lo notaras, nuestro set de instrucciones no tiene `POP`.
`POP` en nuestro compilador solo seria utilizado cuando una función devuelve,
Lo cual es como:

```
; recupera el marco anterior
mov    esp, ebp
pop    ebp
; return
ret
```

De esta manera vamos a hacer otra instrucción `LEV` para conseguir el trabajo
de `MOV`, `POP` y `RET`:

```c
else if (op == LEV) {sp = bp; bp = (int *)*sp++; pc = (int *)*sp++;} // recuprta marco de llamada y PC
```

### LEA

Las instrucciones introducidas arriba intentan resolver el problema de
crear/destruir marcos de llamada, nos queda como obtener los argumentos
dentro de la sub función.

Pero vamos a echar un vistazo al aspecto de un marco de llamada antes de
aprender como tomar argumentos (notese que los argumentos estan siendo
empujados en el orden que se llaman):

```
sub_function(arg1, arg2, arg3);

|     ...            |   dirección alta
+--------------------+
|    arg: 1          |   new_bp + 4
+--------------------+
|    arg: 2          |   new_bp + 3
+--------------------+
|    arg: 3          |   new_bp + 2
+--------------------+
| devolver dirección |   new_bp + 1
+--------------------+
|    old BP          |  <- new BP
+--------------------+
|    local var 1     |   new_bp - 1
+--------------------+
|    local var 2     |   new_bp - 2
|     ...            |   dirección baja
```

Asique si necesitamos referirnos a `arg1`, necesitamos tomar `new_bp + 4`,
lo cual no puede ser conseguido con nuestra mediocre instrucción `ADD`.
De esta manera vamos a hacer otro `ADD` especial para hacer esto:
`LEA <offset>`.

```c
else if (op == LEA) {ax = (int)(bp + *pc++);} // load address for arguments
```

Junto a las instrucciones de arriba podemos hacer llamadas a funciones.

### Instrucciones matemáticas

Nuestra VM tendra una instrucción por cada operador en C. CAsa operador tiene
dos argumentos: el primero es almacenado en la cima del stack, mientras que el
segundo es almacenado en `AX`. El order importa, especialmente con operadores
como `-` o `/`. Al terminar el computo, el stack sera extraido y el resultado
sera almacenado en `AX`. Asique si no eres capaz de extraer el primer
argumento del stack porfavor tenlo en cuenta.

```c
else if (op == OR)  ax = *sp++ | ax;
else if (op == XOR) ax = *sp++ ^ ax;
else if (op == AND) ax = *sp++ & ax;
else if (op == EQ)  ax = *sp++ == ax;
else if (op == NE)  ax = *sp++ != ax;
else if (op == LT)  ax = *sp++ < ax;
else if (op == LE)  ax = *sp++ <= ax;
else if (op == GT)  ax = *sp++ > ax;
else if (op == GE)  ax = *sp++ >= ax;
else if (op == SHL) ax = *sp++ << ax;
else if (op == SHR) ax = *sp++ >> ax;
else if (op == ADD) ax = *sp++ + ax;
else if (op == SUB) ax = *sp++ - ax;
else if (op == MUL) ax = *sp++ * ax;
else if (op == DIV) ax = *sp++ / ax;
else if (op == MOD) ax = *sp++ % ax;
```

### Intrucciones de fábrica

Aparte de la lógica vital, el programa necesita mecanismos de entrada/salida
para poder interactuar con el usuario. `printf` en C es una de las funciones
de salida comunmente más usadas. `printf` es muy complejo de implementar,
pero inevitable si queremos que nuestro compilador sea bootstrapping.

Nuestro plan va a ser crear un puente de nuevas intrucciones entre el programa
y el interprete. De tal manera podremos utilizar bibliotecas del sistema.

Necesitaremos `exit`. `open`, `close`, `read`, `printf`, `malloc`, `memset` y
`memcmp`:

```c
else if (op == EXIT) { printf("exit(%d)", *sp); return *sp; }
else if (op == OPEN) { ax = open((char*)sp[1], sp[0]); }
else if (op == CLOS) { ax = close(*sp); }
else if (op == READ) { ax = read(sp[2],  (char*)sp[1], *sp); }
else if (op == PRTF) { tmp = sp + pc[1]; ax = printf((char*)tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5], tmp[-6]); }
else if (op == MALC) { ax = (int)malloc(*sp); }
else if (op == MSET) { ax = (int)memset((char*)sp[2], sp[1], *sp); }
else if (op == MCMP) { ax = (int)memcmp((char*)sp[2], (char*)sp[1], *sp); }
```

Finalmente, ayadiremos un poco de manejo de errores:

```c
else {
  printf("Instrucción desconocida:%d", op);
  return -1;
}
```

## Test

Vamos a hacer algo de "programación en ensamblador" para calcular `10 + 20`:

```c
int main(int argc, char **argv) {
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

Compila el interprete con `gcc xc-tutor.c` y ejecutalo con `./a.out hello.c`,
conseguí el siguiente resultado:

```
exit(30)
```

Notese que especificamos `hello.c` pero realmente no es usado, lo necesitamos
porque el interprete que construimos en el capítulo anterior necesita un
archivo para funcionar.

Bueno, parece que nuestra VM funciona correctamente :)

## Resumen

Aprendimos como el ordenador funciona internamente y contruimos nuestro propio
set de instrucciones diseñado en base a instrucciones assembly `x86`. Vamos a
intentar a aprender assembly y como funciona construyendo nuestra propia
versión.

El código para este capítulo puede ser descargado desde
[GitHub](https://github.com/lotabout/write-a-c-interpreter/tree/step-1), o
clonandolo con:

```
git clone -b step-1 https://github.com/lotaboyt/write-a-C-interpreter
```

Notese que añadir nuevas intrucciones a un ordenador requeriria diseñar nuevos
circuitos, lo cual es muy costoso. Pero es casi gratuito añadir nuevas
intrucciones a nuestra VM. Estamos tomando ventaja se esto al separar las
funciones de una instrucción en varas para simplificar la implementación.

Si estas interesado, construye tu propio set de instrucciones!
