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
