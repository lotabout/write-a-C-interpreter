> El análisis léxico es el proceso de convertir una secuencia de caracteres
> (tal como un programa de ordenador o una página web) en una secuencia de
> tokens (texto con un sentido identificado).

Normalmente representamos los tokens como pare: `(token type, token value)`.
Por ejemplo, si la funte de un programa contiene la cadena: "998", el lexógrafo
lo tratara como si fuese el token `(Number, 998)` significando que es un numero
con el valor de `998`.

## Lexógrafo vs Compilador

Primero miremos la estructura de un compilador:

```
                    +-----------+                       +--------+
-- código funte --> | lexógrafo | --> cadena tokens --> | parser | --> assembly
                    +-----------+                       +--------+
```

El compilador se puede considerar un programa que transforma C en assembly.
En este sentido, el lexógrafo y el parser son transformadores tambien:
El lexógrafo toma código fuente y lo tranforma en una cadena de tokens;
El parser consumen la cadena y la transforma en assembly.

¿Entonces, porque necesitamos un lexógrafo y un parser? Esque el trabajo de
compilador es complicado. Asquique necesitamos reclutar el lexógrafo y el
parser para que hagan el trabajo, de esta manera se simplifica la tarea.

Ese es el valor del lexógrafo: simplificar el parser convirtiendo el codigo
fuente en token.

## Decisión de Implementación

Antes de empezar quiero que sepas que hacer un lexer es aburrido y tendiente a
errores. Esa es la razón por la que genios hayan creado herramientas de
automaticaciñon para hacer el trabajo. `lex/flex` nos permiten describir las
reglas léxicas utilizando expresiones regulares para que generen el lexógrafo
por nosotros.

Notese que no vamos a seguir el gráfico de arriba, es decir, no vamos a
convertir todo el codigo fuente en tokens de una. Las razones son:

1. Convertir el código fuente en tokens tiene contexto. Como una cadena es
   interpretada depende de donde aparezca.
2. Es una perdida de recursos almacenar todo los tokens proque solo seran
   utilizados unos pocos al mismo tiempo.

De esta manera implementaremos una función: `next()` que devuelve un token cada
vez que es llamado.

## Tokens admitidos

Añade la siguiente definición en el espacio global:

```c
// tokens and classes (operators last and int precedence order)
enum {
  Num = 128, Fun, Sys, Glo, Loc, Id,
  Char, Else, Enum, If, Int, Return, Sizeof, While,
  Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak
};
```

Todos estos son tokens que nuestro lexógrafo pude entender. El lexógrafo
interpretara cadenas como `=` en tokens `Assign`; `==` como `Eq`; etc.

Como un token puede contener más de un caracter el lexógrafo reduce la
complejidad. El parser ya no necesita analizar varios caracteres para
identificar el significado de una cadena.

Por supuesto los tokens estan ordenados apropiadamente reflejando su prioridad
en el lenguaje C. `*(Mul)` por ejemplo tiene más prioridad que `+(Add)`.
Hablaremos de esto más adelante.

Finalmente hay algunos caracteres que no necesitamos incluirlos como tokens,
por ejemplo `]` o `~`. La razón por la que los codificamos distinto son:

1. Estos tokens contienen un solo caracter, de esta manera son más faciles de
   identificar.
2. No estan invueltos en combates de prioridad.

## Esqueleto del lexógrafo

```c
void next() {
    char *last_pos;
    int hash;
    while (token = *src) {
	    ++src;
        // aquí se parsean los tokens
    }
    return;
}
```

¿Por qué necesitamos `while` aquí sabiendo que `next` solo leera un token?
Esto causa preguntas en el desarrollo de compiladores (recuerda que el lexógrafo es un tipo de compilador): ¿Como manejamos un error?

Normalmente hay dos soluciones viables:

1. Señala donde ocurrió el error y termina.
2. Señala donde ocurrió el error, es lo salta, y continua.

Eso explica el uso de `while`: para saltarnos carácteres desconocidos en el
código. Ademas tambien se utiliza para comernos los espacios, que aunque sean
parte del código, no existen en el programa.

## Saltos de linea

Los macros en C empiezan con un hashtag `#`, como por ejemplo
`#include <stdio.h>`. Nestro compilador no admite ningún macro, así que nos
los vamos a saltar:

```c
else if (token == '#') {
    // saltar macro porque no los admitimos
    while (*src != 0 && *src != '\n') {
        src++;
    }
}
```

## Identificadores y tabla de símbolos

Un identificador es el nombre de una variable. No obstante al lexógrafo le dan
igual los nombres, solo le importa la identidad. Por ejemplo: `int a`
declara un variable; tenemos me saber que ` = 10;` se refiere a la variable
que le precede.

Basandonos en este razonamiento seremos capaz de almacenar todos los nombres
que hemos encontrado en algo que llamaremos tabla de símbolos. Miraremos la
tabla cuando encontremos un nombre. Si el nombre existe en la tabla la
identidad será devuelta.

Entonces ¿Comó vamos a representar la identidad?

```c
struct identifier {
    int token;
    int hash;
    char *name;
    int class;
    int type;
    int value;
    int Bclass;
    int Btype;
    int Bvalue;
}
```

Necesitaremos una explicación aquí:

1. `token`: es el token del identificador. En teoría debería de estar adjunto
   al tipo `Id`. Pero esto no se cumple pues añadiremos semanticas tales como
   `if` o `while` los cuales son un identificador especial.
2. `hash`: el hash del nombre, para optimizar la comparación en la tabla.
3. `name`: el nombre en cuestión.
4. `class`: Depende si el identificador es global, local, o constante.
5. `type`: el tipo del identificador, `int`. `char` o pointer.
6. `value`: el valor de la variable almacenado en el identificador.
7. `Bxxxx`: variable local que le hace shadowing a la variable global. Usado
   usado para almacenar la variable global si ocurre.
   
Una tabla de simbolos tradicional solo contendría el `name` mientras que
nuestra tabla también almacenara información que solo sera leido por el parser.

Desgraciadamente nuestro compilador no admite `struct`, y estamos intentado
hacer bootstrapping. Asique hay que comprometer la estructura del identificador.

```
Tabla de símbolos:
----+-----+----+----+----+-----+-----+-----+------+------+----
 .. |token|hash|name|type|class|value|btype|bclass|bvalue| ..
----+-----+----+----+----+-----+-----+-----+------+------+----
    |<---       one single identifier                --->|
```

Eso significa que utilizaremos un único array de `int` para almacenar toda
la información. Cada ID utilizará 9 celdas. El código es el siguiente:

```c
int token_val;                // valor del token actual
int *current_id,              // ID siendo parseado.
    *symbols;                 // Tabla de simbolos
// Campos del identificador
enum {Token, Hash, Name, Type, Class, Value, BType, BClass, BValue, IdSize};
void next() {
        ...
        else if ((token >= 'a' && token <= 'z') || (token >= 'A' && token <= 'Z') || (token == '_')) {
            // Identificador del parser
            last_pos = src - 1;
            hash = token;
            while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') || (*src == '_')) {
                hash = hash * 147 + *src;
                src++;
            }
            // buscar identificadores actuales, busqueda lineal
            current_id = symbols;
            while (current_id[Token]) {
                if (current_id[Hash] == hash && !memcmp((char *)current_id[Name], last_pos, src - last_pos)) {
                    //found one, return
                    token = current_id[Token];
                    return;
                }
                current_id = current_id + IdSize;
            }
            // almacenar nuevo ID
            current_id[Name] = (int)last_pos;
            current_id[Hash] = hash;
            token = current_id[Token] = Id;
            return;
        }
        ...
}
```

Notese que la busqueda el la tabla es lineal y podria ser mejorado.

## Numeros

Necesitamos admitir numeros deciamles, hexadecimales, y octales. La lógica es
bastante simple excepto como obtener el valor hexademical.

```c
token_val = token_val * 16 + (token & 0x0F) + (token >= 'A' ? 9 : 0);
```

En caso de que no estes familiarizado con estos pequeños trucos, `a` es el 
valor hexademical de `61` mientras que `A` es `41`. Asi que `toke & 0x0F`
puede obtener el digito minúscula del caracter.

```c
void next() {
        ...



        else if (token >= '0' && token <= '9') {
            // parsear numero, tres tipos: dec(123) hex(0x123) oct(017)
            token_val = token - '0';
            if (token_val > 0) {
                // dec, empiza con [1-9]
                while (*src >= '0' && *src <= '9') {
                    token_val = token_val*10 + *src++ - '0';
                }
            } else {
                // empieza con 0
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

## Literales de texto

Si encontramos un literal, necesitamos almacenarlo en `data segment`, el cual
introducimos en un capítulo pasado, y devolver el adress. Otro problema es que
necesitamos encargarnos de carácteres escapados como `\n` para representar
saltos de linea. Pero no admitimos escapes otros que `\n` porque no importan
para bootstrapping. Notese que igualemente admitiremos escapes como `\x`
tratandolos como un simple `x` en este caso.

Nuestro lexógrafo analizará un soloc caracter (p.e. `'a'`) al mismo tiempo.
Una vez el caracter es encontrado, lo devolveremos como un `Num`.

```c
void next() {
        ...

        else if (token == '"' || token == '\'') {
            // parsear string literal, currently, por ahora el único escape admitido
            // Es '\n', almacenar string literal e data.
            last_pos = data;
            while (*src != 0 && *src != token) {
                token_val = *src++;
                if (token_val == '\\') {
                    // escape character
                    token_val = *src++;
                    if (token_val == 'n') {
                        token_val = '\n';
                    }
                }
                if (token == '"') {
                    *data++ = token_val;
                }
            }

            src++;
            // Si es un solo caracter, devolver Num
            if (token == '"') {
                token_val = (int)last_pos;
            } else {
                token = Num;
            }

            return;
        }
}
```

## Comentarios

Solo los comentarios al estilo de C++ son admitidos `//`, los del estilo C no
`/**/`.

```c
void next() {
        ...

        else if (token == '/') {
            if (*src == '/') {
                // saltar comentarios
                while (*src != 0 && *src != '\n') {
                    ++src;
                }
            } else {
                // operador de división
                token = Div;
                return;
            }
        }

        ...
}
```

Ahora introduciremos el concepto de `lookahead`: En el código de arriba
podemos ver que el código empieza con el carácter `'\'`, entonces
encontraremos un comentario o una división.

A veces no podemos decidir que token generar solo con el carácter actual, asi
que necesitamos mirar el siguiente (`lookahead`) para determinar el resultado.
En nuestro caso, si es otro slash `/` sera un comentario, sino, una división.

Como lo hemos mencionado anteriormente, un lexógrafo y parser son compiladores,
en el parser también encontraremos `lookahead` pero en vez de mirar carácteres
mirara tokens. El `k` en `LL(k)` en teoría de compiladores es la cantidad de tokens que un parser necesita mirar.

(nota del editor: Estoy cansado de escribir `lexógrafo`, en ahora en adelante
solo utilizare `lexer`)
Si no separamos el lexer y el parser, el compilador necesitara leer muchos más
carácteres para determinar el siguiente paso. Pues podemos decir que el lexer
reduce la carga del parser.

## Otros

Los otros son claros y concisos, compruebe el código:

```c
void next() {
        ...
        else if (token == '=') {
            // parse '==' y '='
            if (*src == '=') {
                src ++;
                token = Eq;
            } else {
                token = Assign;
            }
            return;
        }
        else if (token == '+') {
            // parse '+' y '++'
            if (*src == '+') {
                src ++;
                token = Inc;
            } else {
                token = Add;
            }
            return;
        }
        else if (token == '-') {
            // parse '-' y '--'
            if (*src == '-') {
                src ++;
                token = Dec;
            } else {
                token = Sub;
            }
            return;
        }
        else if (token == '!') {
            // parse '!='
            if (*src == '=') {
                src++;
                token = Ne;
            }
            return;
        }
        else if (token == '<') {
            // parse '<=', '<<' o '<'
            if (*src == '=') {
                src ++;
                token = Le;
            } else if (*src == '<') {
                src ++;
                token = Shl;
            } else {
                token = Lt;
            }
            return;
        }
        else if (token == '>') {
            // parse '>=', '>>' o '>'
            if (*src == '=') {
                src ++;
                token = Ge;
            } else if (*src == '>') {
                src ++;
                token = Shr;
            } else {
                token = Gt;
            }
            return;
        }
        else if (token == '|') {
            // parse '|' o '||'
            if (*src == '|') {
                src ++;
                token = Lor;
            } else {
                token = Or;
            }
            return;
        }
        else if (token == '&') {
            // parse '&' y '&&'
            if (*src == '&') {
                src ++;
                token = Lan;
            } else {
                token = And;
            }
            return;
        }
        else if (token == '^') {
            token = Xor;
            return;
        }
        else if (token == '%') {
            token = Mod;
            return;
        }
        else if (token == '*') {
            token = Mul;
            return;
        }
        else if (token == '[') {
            token = Brak;
            return;
        }
        else if (token == '?') {
            token = Cond;
            return;
        }
        else if (token == '~' || token == ';' || token == '{' || token == '}' || token == '(' || token == ')' || token == ']' || token == ',' || token == ':') {
            // devolver el carácter como token
            return;
        }

        ...
}
```

## Palabras clave y funciones nativas

Las palabras clave como `if`, `while` o `return` son especiales porque son
conocidos por el compilador desde el inicio. No podemos tratarlos como
identificadores normales porque tienen un significado especial. Tenemos dos
maneras de manejarlos:

1. Dejar al lexer parsearlos y devolver el token que los identifica.
2. Tratarlos como identificadores normales pero seran almacenados en la tabla
   de antemano.
   
Vamos a escojer la segunda opción: añadir los identificadores correspondientes
en la tabla y ponerles los propiedades mencionadas. Entonces cuando una de
estas es encontrado en el código sera interpretado como un identificador,
pero como ya existía sabremos que es diferente a los demas.

Con las funciones nativas ocurre algo parecido. Solo seran diferentes en la
información interna.

```c
// tipo de variable/función
enum { CHAR, INT, PTR };
int *idmain;                  // la función 'main'
int main(int argc, char **argv) {
    ...

    src = "char else enum if int return sizeof while "
          "open read close printf malloc memset memcmp exit void main";


    // añadir palabras clave a la tabla
    i = Char;
    while (i <= While) {
        next();
        current_id[Token] = i++;
    }

    // añadir la libreria a la tabla
    i = OPEN;
    while (i <= EXIT) {
        next();
        current_id[Class] = Sys;
        current_id[Type] = INT;
        current_id[Value] = i++;
    }

    next(); current_id[Token] = Char; // manejar el tipo void
    next(); idmain = current_id; // seguir main

    ...
    program();
    return eval();
}
```

## Código

Puedes encontrar el codigo en
[Github](https://github.com/lotabout/write-a-C-interpreter/tree/step-2), o
clonarlo con:

```
git clone -b step-2 https://github.com/lotabout/write-a-C-interpreter
```

Ejecutar el código resultara en un `Segmentation Fault` por que tratara de
ejecutar la máquina virtual que contruimos en el capítulo anterior que no
contiene ningún código ejecutable.

## Resumen

1. El lexer se usa para pre-procesar el código, para reducir la complejidad
   del parser.
2. El lexer es un tipo de compilador que comsume código y crea una cadena de
   tokens.
3. `lookahead(k)` se usa para determinar el significado del token/carácter
   actual.
4. Los identificadores son representados en la tabla de símbolos.

Vamos a tratar que es `top-down recursive parser`. Hasta entonces :)
