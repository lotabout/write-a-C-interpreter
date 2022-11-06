Esta serie de artículos es un tutorial para contruir un compilador de C
desde cero.

Menti un poquito en la frase superior: en realidad es un _interpretador_ en
vez de un _compilador_. Mentí porque ¿Qué demonios es un "interpretador de
C"? Sin embargo, entenderas mejor los compiladores construyendo un
interpretador.

Claro, deseo que puedas conseguir un entendimiento basíco de como un
compilador es construido, y darte cuenta de que no es tan difícil
constuir uno. ¡Buena suerte!

Finalmente, esta serie se escribio originalmente en Chino, sientete libre
eb corregirme si te sientes confundido por mi inglés. Y apreciaría mucho
si pudieras enseñarme algo de inglés "nativo" :)
(Nota del traductor: Yo ne he traducido esta serie del chino al inglés,
sino del inglés al español.)

No vamos a escribir ningún codigo en este capítulo, asique sientete libre
de saltartelo si estas desesperado para ver algo de codigo...

## ¿Por qué deberiás preocuparte con teoría de compiladores?
p
¡Porque es **GUAY**!

Y es muy util. Los programas son contruidos para hacer algo por nosotros,
cuanfo son usados para transformar un tipo de data en otro, los llamamos
compiladores. De esta manera, aprendiendo teoria de compiladores estamos
intentando dominar una muy útil y potente técnica para resolver problemas.
¿No es eso suficientemente guay para tí?

La gente solía decir que entender como un compilador funciona te permitiría
escribir mejor código. Algunos argumentaran que los compiladores modernos
son tan buenos en optimizar que ya no deberíad preocuparte. Bueno, eso es
cierto, la mayoría no necesita aprender teoría de compiladores solo para
mejorar la eficiencía del codigo. Y por la mayoría, ¡Me refiero a tí!

## A nosotros tampoco nos gusta la teoría

Siempre he admirado la teoría de compilar porque eso es lo que hace la
programación fácil. De qualquier modo, ¿Puedes imaginarte contruir un
navegador web solo en assembly? Asique cuando conseguí una oportunidad
para aprender teoría de compilar en la facultad, ¡Estaba tan emocionado!
I luego... Lo dejé, no entendiendo el que.

Normalmente, por supuesto, un curso sobre compiladores cubrirá:

1. Como representar sintaxis (e.g.: BNF, etc.)
2. Lexografo, con algo de NFA (Nondeterministic Finite Automata / Automata
   Finito Nodeterminante), DFA (Deterministic Finite Automata / Automata
   Finito Deterministico).
3. Analidor, tal como descenso recursivo, LL(k), LALR, etc.
4. Lenguajes intermediarios.
5. Generación de código.
6. Optimización de código.

Tal vez a más del 90% de estudiantes no les importe en nada más alla del
analizador, y aún más, !Todavía no sabemos como construir un compilador!
Despues de todo el esfuerzo en aprender las teorías. Bueno, la razón
principal es que "Teoria de compiladores" intenta enseñar "Como construir
un generador de analizadores", en pocas palabras una herramienta que consume
sintaxis y gramática y genera un compilador por tí. lex/yacc o flex/bison
o cosas como esas.

Estas teorias intentan enseñarnos como resolver problemas generales de
generación de compiladores automáticamente. Eso significa que una vez
que los domines, seras capaz de trabajar con todo tipo de gramáticas. Son,
en efecto útiles en la industria. Sin embargo, son demasiado poderosos y
complejos para estudiantes y la mayoría de programadores. Entenderas eso
si intentas leer codigo fuente lez/yacc.

Las buenas noticias son que contruir un compilador puede ser mucho más fácil
que nunca te ayas imaginado. No mentire, no es fácil, pero definitivámente
no es difícil.

## Nacimiento de este proyecto

Un día me cruzé con el proyecto [c4](https://github.com/rswier/c4) en GitHub.
Es un pequeño interprete en el cual se proclama solo ser implementado en
cuatro funciones. La parte más sorprendente es que "bootstrapping" (que se
interpreta a si mismo). ¡También es implementado en alrededor de 500 lineas!

Mientras tanto he leido muchos tutoriales sobre compiladores, son o muy
simples (tal como implementar una simple calculadora) o utilizan
herramientas de automación (tales como flex/bison). c4 es, sin embargo,
implementado desde cero. Lo triste es que intenta ser mínimo, eso hace el
codigo bastante rebuscado, y difícil de entender. Asique he empezado un
nuevo proyecto tambien:

1. Implementar un compilador de C (en relidad, interprete)
2. Escribit un tutorial de como es construido.

Me tomo una semana rescribirlo, resultando en 1400 de lineas incluyendo
comentarios. El proyecto es hosteado en GitHub [Write a C interpreter](https://github.com/lotabout/write-a-C-interpreter)

¡Gracias a rswier por traernos un maravilloso proyecto!

## Antes de que te vallas

Implementar un compilador puede ser aburrido y difícil de debuggear. Asique
espero que puedas invertir sufuciente tiempo estudiando, tanto como
escribiendo el codigo. Estoy seguro que sentiras una gran sensación de
logro tal como you lo hago.

## Buenos recursos

1. [Let's Build a Compiler](http://compilers.ieec.com/crenshaw/): un muy
   buen tutorial para contruir un compilador para novatos.
2. [Lemon Parser Generator](http://www.hwaci.com/sw/lemon/): el generador
   de analizadores que es usado en SQLite. Buena lectura si quieres
   entender teoria de compiladores con codigo.
   
Al final del dia, soy un humano con un nivel general, habran inevitablemente
errores con los articulos y el código (también mi inglés). ¡Sientete libre
de corregirme! (nota del traductor: aunque yo sea español y tenga un alto
nivel de ingles, cometere errores. Si quieres notificarme sobre ellos manda
un mensaje a mi email greenerclay@gmail.com, o haz una PR tu mismo)

Espero que lo disfrutes.
