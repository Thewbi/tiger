# Building

First, make sure that the command yacc is installed on your system

```
$ yacc
'yacc' is not recognized as an internal or external command,
operable program or batch file.
```

If yacc is not present, check if bison is present.

```
$ bison
bison: missing operand
Try 'bison --help' for more information.
```

If you have bison, update the makefile to use bison instead of yacc.

You need to copy the tiger.lex file from chapter 2 into chapter 3.

Inside tiger.lex, replace token.h by y.tab.h

```
//#include "tokens.h"
#include "y.tab.h"
```

The general build order is as follows:

The parser generator yacc/bison will read the tiger.grm grammar definition.
It will assemble a list of all required token and their id number and output that list to y.tab.h.
This file is included into the lexer (tiger.lex) so that the lexer generator lex/flex knows which
ids to use for the token it emits. When the parser has defined the token id's and the lexer emits
exactly those token id's then both generated parts will work together nicely. This is the reason why
the parser is generated before the lexer although at runtime lexing has to be performed first to 
generate input for the parser!
The parser generator will also generate the parser C source code and write it into y.tab.c

Next flex is called on tiger.lex.
It will generate the lexer and store it into lex.yy.c

Next the compiler will produce .o files from parsetest.c, y.tab.c, lex.yy.c, errormsg.c and util.c

The executable parsetest is linked from all .o files.

### Running parsetest.exe

```
parsetest.exe ..\testcases\test1.tig
```

Initially, without ever updating the tiger.grm grammar definition, the parsing of any .tig file
will fail because no valid grammar production rules are contained in tiger.grm.

### Writing the grammar

The task in chapter 3 is to create a complete grammar definition for the tiger language.

Bison is used as a parser generator. Bison reads grammar files (tiger.grm) as an input and it
will generate a list of token along with their ids and a parser that consume tokens that a lexer
feeds into it.

The tiger.grm file contains rules for each production in the grammar.
The productions are defined in Appendix starting on page 518.

Sample test1.tig looks like this:

```
/* an array type and an array variable */
let
	type  arrtype = array of int
	var arr1:arrtype := arrtype [10] of 0
in
	arr1
end
```

Let's create a grammar that is able to parse test1.tig

We see a let-in-end construct as the outermost construct.
Searching the appendix, the section on expressions is contained on page 522.
It is contained within A.3 VARIABLES AND EXPRESSIONS

The definition of let is given amongst the list of all expressions:

```
Let: The expression let decs in expseq end evaluates the declarations decs, binding types, variables, 
and procedures whose scope then extends over the expseq. 

The expseq is a sequence of zero or more expressions, separated by semicolons. 
The result (if any) of the last exp in the sequence is then the result of the entire let-expression.
```

let itself is an expression since it is listed under the section on expressions.
It's structure is defined to be 

```
let decs in expseq end
```
where let, in and end are terminals which the lexer is actually able to identify and will return token for.
The parts decs and expseq are not terminals but they are themselves constructed from several terminals.
Therefore further productions have to be provided, one for decs and one for expseq.

decs is defined within A.2 DECLARATIONS

```
decs → {dec}
```

So decs is a list of one or more dec.

dec is defined to be

```
dec → tydec
    → vardec
    → fundec
```

dec is either a type declaration, a variable declaration or a function declaration.

It test1.tig, there is one type declaration and one variable declaration

tydec is defined to be 

```
tydec → type type-id = ty

ty → type-id
   → { tyfields } (these braces stand for themselves)
   → array of type-id

tyfields → ϵ
         → id : type-id {, id : type-id}
```

vardec is

```
vardec → var id := exp
       → var id : type-id := exp
```

expseq is an expression sequence

It is defined to be

```
The expseq is a sequence of zero or more expressions, separated by semicolons.

Sequencing: A sequence of two or more expressions, surrounded by parentheses and separated by semicolons (exp;exp; ... exp) evaluates all the expressions in order. The result of a sequence is the result (if any) yielded by the last
of the expressions

expseq → {exp SEMICOLON}
```

The expression in test1.tig is just arr1

arr1 will be evaluated to an l-value

```
lvalue → id
       → lvalue . id
       → lvalue [ exp ]
```

and specifically to an id within the lvalue production.

id is the id token that the lexer returns. It is defined to be:

```
Identifiers: An identifier is a sequence of letters, digits, and underscores, starting with a letter. Uppercase letters are distinguished from lowercase. In this
appendix the symbol id stands for an identifier.
```
