Esta série de artigos é um tutorial para construir um compilador C do zero.

Menti um pouco na frase acima: é na verdade um interpretador em vez de compilador. Menti porque, afinal, o que é um "interpretador C"? No entanto, você entenderá melhor os compiladores construindo um interpretador.

Sim, espero que você possa ter uma compreensão básica de como um compilador é construído e perceba que não é tão difícil construir um. Boa sorte!

Por fim, esta série foi escrita originalmente em chinês. Sinta-se à vontade para me corrigir se estiver confuso com o meu inglês. E eu ficaria muito grato se você pudesse me ensinar um pouco de inglês "nativo" :)

Não escreveremos nenhum código neste capítulo, sinta-se à vontade para pular se estiver ansioso para ver algum código...

## Por que você deve se importar com a teoria dos compiladores?

Porque é INCRÍVEL!

E é muito útil. Programas são construídos para fazer algo por nós; quando são usados para traduzir uma forma de dados para outra, podemos chamá-los de compilador. Assim, ao aprender um pouco sobre a teoria dos compiladores, estamos tentando dominar uma técnica muito poderosa de resolução de problemas. Isso não é incrível para você?

As pessoas costumavam dizer que entender como um compilador funciona ajudaria você a escrever um código melhor. Alguns argumentariam que os compiladores modernos são tão bons em otimização que você não deveria mais se preocupar. Bem, é verdade, a maioria das pessoas não precisa aprender a teoria dos compiladores apenas para melhorar a eficiência do código. E por maioria das pessoas, quero dizer você!

## Também Não Gostamos de Teoria

Sempre fiquei impressionado com a teoria dos compiladores porque é isso que torna a programação fácil. De qualquer forma, você consegue imaginar construir um navegador web apenas em linguagem assembly? Então, quando tive a chance de aprender a teoria dos compiladores na faculdade, fiquei tão animado! E então... desisti, sem entender nada.

Normalmente, um curso de compilador cobrirá:

* Como representar sintaxe (como BNF, etc.)

* Lexer, com algo como NFA (Autômato Finito Não Determinístico), DFA (Autômato Finito Determinístico).

* Parser, como descida recursiva, LL(k), LALR, etc.

* Linguagens Intermediárias.

* Geração de código.

* Otimização de código.

Talvez mais de 90% dos alunos não se importem com nada além do parser e, além disso, ainda não sabemos como construir um compilador! Mesmo depois de todo o esforço para aprender as teorias. Bem, a principal razão é que o que a "Teoria do Compilador" tenta ensinar é "Como construir um gerador de parser", ou seja, uma ferramenta que consome gramática de sintaxe e gera um compilador para você. Como lex/yacc ou flex/bison.

Essas teorias tentam nos ensinar como resolver os problemas gerais de geração de compiladores automaticamente. Isso significa que, uma vez que você os tenha dominado, será capaz de lidar com todos os tipos de gramáticas. Eles são realmente úteis na indústria. No entanto, são muito poderosos e complicados para estudantes e a maioria dos programadores. Você entenderá isso se tentar ler o código-fonte do lex/yacc.

A boa notícia é que construir um compilador pode ser muito mais simples do que você imagina. Não vou mentir, não é fácil, mas definitivamente não é difícil.

## Nascimento deste projeto 

Um dia me deparei com o projeto c4 no Github. É um pequeno interpretador C que se diz ser implementado por apenas 4 funções. A parte mais incrível é que ele é auto-suficiente (interpreta a si mesmo). Além disso, é feito com cerca de 500 linhas!

Ao mesmo tempo, li muitos tutoriais sobre compiladores, eles são ou muito simples (como implementar uma calculadora simples) ou usam ferramentas automáticas (como flex/bison). O c4, no entanto, é implementado do zero. A parte triste é que ele tenta ser minimalista, o que torna o código uma bagunça, difícil de entender. Então, comecei um novo projeto para:

* Implementar um compilador C funcional (na verdade, um interpretador)
* Escrever um tutorial de como ele é construído.

Levei 1 semana para reescrevê-lo, resultando em 1400 linhas, incluindo comentários. O projeto está hospedado no Github: Escreva um Interpretador C.

Obrigado rswier por nos trazer um projeto maravilhoso!

## Antes de você ir

Implementar um compilador pode ser entediante e é difícil de depurar. Então, espero que você possa dedicar tempo suficiente para estudar, bem como digitar o código. Tenho certeza de que você sentirá um grande senso de realização, assim como eu.

## Bons Recursos

* Vamos Construir um Compilador: um tutorial muito bom para construir um compilador para iniciantes.

* Lemon Parser Generator: o gerador de parser usado no SQLite. Bom para ler se você quer entender a teoria dos compiladores com código.

No final, sou humano com um nível geral, haverá inevitavelmente erros nos artigos e códigos (e também no meu inglês). Sinta-se à vontade para me corrigir!

Espero que você goste.
 