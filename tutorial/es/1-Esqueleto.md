En este capítulo vamos a tener un resumen de la estructura de un compilador.

Antes de empezar, em gustaria reafirmar que lo que queremos es un construir
es un **interprete**. Eso significa que podremos ejecutar código C como
si fuera un script. Es elejido principalmente por dos razones:

1.  El interprete difiere del compilador solo en la fase de generación
    de código, así aun aprenderemos todas ls técnicas vitales para contruir
    un compilador (tales como analisis léxico y interpretación).
2.  Vamos a contruir nuestra propia máquina virtual y instrucciónes de
    ensamblador, eso nos ayudaŕa a entender como los ordenadores funcionan.

## Tres fases

Provisto de un archivo fuente, el compilador ejecutara tres fáses de
procesamiento:

1.  Análisis léxico: convierte las cadenas de texto fuente en identificadores
    internos.
2.  Interpretación: consume los identificadores y construye un arbol sintáctico.
3.  Generación de código: camina atraves del arbol sintáctico y genera código
    para la plataforma deseada.

La contrucción de compiladores ha madurado tanto que la parte 1 y 2 pueden ser
hechas por herramientas de automaticación. Por ejemplo, flex puede ser usado
para análisis léxico, bison para interpretación. Son herramientas poderosas
pero hacen miles de cosas detras de las cortinas. Para totalmente entender
como contruir un compilador vamos ha contruirlo desde cero.

De esta manera vamos a contruir un interprete en los siguientes pasos:

1.  Construir nuestra propia máquina virtual y set de instrucciones. Esta
    va ser nuestra plataforma objetivo que vamos a utilizar para nuestra
    fase de generación de código.
2.  Construir nuestro propio lexógrafo para el compilador C.
3.  Escribir un interpretador de descenso recursivo propio.

## Esqueleto de nuestro compilador

Modelando en base a c4, nuestro compilador contiene cuatro funciones
principales:

1.  `next()` para análisis léxico; conseguir el sugiente indetificador;
    ignorará los espacios, tabuladores, etc.
2.  `program()` entrada principal para el interpretador.
3.  `expresión(level)`: interpretador de expresiones; `level` sera explicado
    en el seguiente cápitulo.
4.  `eval()`: la entrada de la máquina virtual; usado para interpretar las
    instrucciones.

¿Por qué existe `expression` cuando tenemos `program` para interpretadores?
Eso es porque el interpretador de expresiones es relativamente independiente
y complejo, asique lo ponemos en único modulo (función).

El código es el siguiente:
```c
#include <stdio.h>
#include <stdlin.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#define int long long  // trabaja con objetivos 64bit
 
int token;             // identificador actual
char *src, *old_src;   // punteros a cadenas de código;
int poolsize;          // tamaño defacto de texto/data/stack
int line;              // numero de linea
  
void next() {
  token = *src++;
  return;
}
    
void expression(int level) {
  // nada
}
    
void program() {
  next();
  while (token > 0) {
    printf("El identificador es: %c\n", token);
    next();
  }
}
    
void eval() {
  return 0;
}
    
int main(int argc, char **argv) {
  int i, fd;

  argc--;
  argv++;
 
  poolsize = 256 * 1024;          // Tamaño arbitrario
  line = 1;
  
  fd = open(*argv, 0);
  if (fd < 0) {
    printf("No se pudo abrir(%s)\n", *argv);
    return 1;
  }
    
  old_src = malloc(poolsize);
  src = old_src;
  if (!src) {
    printf("No se pudo allocar(%d) para area fuente\n", poolsize);
    return 1;
  }
    
  // leer archivos fuente
  i = read(fd, src, poolsize - 1);
  if (i <= 0) {
    printf("read() devolvió %d\n", i);
    return 1;
  }
    
  src[i] = 0; // EOF
  close(fd);
  
  program();
    
  return eval();
}
```

Eso es bastante código para el primer cápitulo del árticulo. No obstante
es bastante simple. El código intenta leer un archivo fuente, carácter por
carácter y los escribe.

Ahora mismo el lexografo `next()` no hace nada aparte de devolver los
carácteres tal como aparecen en el código fuente. El analizador `program()`
tampoco hace su trabajo, no se genera ningún arbol sintáctico, no código
objetivo.

Lo importante es entender el significado de estas funciones y como son
relacionados entre si dado a que son el esqueleto de nuestro interprete.
Vamos a rellenarlos pado por paso en proximos capítulos.

## Código

El código de este capítulo se puede descargar desde [GitHub](https://github.com/lotabout/write-a-C-interpreter/tree/step-0), o desde git:
```
git clone -b step-0 https://github.com/lotabout/write-a-C-interpreter
```

Notese que pueda que corrija bugs despues, y si hay alguna inconsistencia
entre los artículos y las ramas de código, sigue el artículo. Solo actualizare
el código el la rama master.

# Resumen

Despues de algo de escritura aburrida, tenemos el más simple de los
compiladores: un haz-nada compilador. En el próximo capítulo vamos a
implementar la función `eval()`, esto es, nuestra propia máquina virtual.
Hasta la próxima.

