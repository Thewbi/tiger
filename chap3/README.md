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

### Understanding the grammar

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

### Writing the grammar in Bison

Bison processes the tiger.grm file and it will generate C code for a parser for the grammar designed in the tiger.grm file.

Working with Bison means adding production rules until all testcases can be parsed.
Actions can be attached to each detected symbol in a rule. Actions are C code that is executed when a symbol
is reduced. In chapter 3 of the book, it is explicitly stated that at this point no actions should be added, so
the grammar will contain none yet.

The largest problem when writing Bison grammars is that of shift/reduce conflicts.
When your grammar creates a shift/reduce conflict, Bison will output a warning.
It will output a warning since shift/reduce conflicts are no fatal errors. Bison will still generate
code for a parser. Bison is able to generate working code under shift/reduce conflicts because
it decides to shift in cases where it could shift and reduce at the same time.
See https://www.gnu.org/software/bison/manual/html_node/Shift_002fReduce.html#:~:text=This%20situation%2C%20where%20either%20a,directed%20by%20operator%20precedence%20declarations.

> This situation, where either a shift or a reduction would be valid, is called a shift/reduce conflict. Bison is designed to resolve these conflicts by choosing to shift, unless otherwise directed by operator precedence declarations.

So what is a shift/reduce conflict exactly.
Grammars contain rules. A rule tells the parser, which symbols (right side of a production rule) can be replaced
by the symbol on the left side of a production rule.
Shifting means that the parser will pull in the next token from the lexer.
The parser will shift in symbols and reduce them until the start symbol is produced. Once the start symbol
is produced, the input has been parsed succesfully.

So at any point in time, the parse can do one of two things. It can reduce a parser rule or it can shift in a new
symbol. There are situations where a parser has the option to reduce or to shift at the same time. This
happens when both options are valid according to the grammar. The grammar is then said to be ambiguous.
Bison outputs a shift/reduce conflict warning and the proceeds with the shift.

The point is that you should get rid of shift/reduce conflicts wherever possible. Do not ignore those warnings.
There are cases where grammars are ambiguous and cannot be rewritten to become unambiguous. But most of the time
you as a grammar designer have the chance to adjust the grammar until it becomes unambiguous.

You have two tools to get rid of ambiguity. Tool 1 is to change your productions. There is no recipe on how
to do this but mostly try to keep the rules simple. If your rules try to achieve too much in one go, the chance
of shift/reduce conflicts rises.

Tool number two is to tell Bison how to handle operator precedences.

Let's assume this grammar:

```
exp : arithmetic
    | INT
    ;

arithmetic : exp PLUS exp
           | exp MINUS exp
           | exp TIMES exp
           | exp DIVIDE exp
           ;
```

Here exp (expression, is a integer or a combination of integers that form a new integer) is recursive
in that two exp can be combined using the operators +, -, * and / to form a new exp.

Imagine the parser sees the following input:

```
1 + 2 + 3
```

The parser current location pointer in the input stream is denoted by a dot and let's say the parser
has just shifted in the symbol 2 and could potentially shift in the symbol + next. The dot mark will
be between 2 and +

```
1 + 2 . + 3
```

From this situation the parser can either reduce 1 + 2 into exp and then shift the plus symbol:

```
exp + . 3
exp + 3 .
exp .
```

or it can shift twice, then reduce 2 + 3 into exp and then reduce 1 + exp into exp

```
1 + 2 + . 3
1 + 2 + 3 .
1 + exp .
exp .
```

The parser has to make a decision on what to do. A stated earlier, Bison will output a warning and
shift by default. The question is how to change the grammar to get rid of the warning and have
a user defined decision making baked into the grammar without the parser performing it's default
conflict resolution strategy!

The solution is this case can be operator precedences.
See https://github.com/FlexW/tiger-compiler/blob/master/src/tiger_grm.y
This tiger grammar makes use of operator precedences to solve the warnings in this case.

```
%nonassoc DO OF
%nonassoc THEN /* ELSE must come after THEN! */
%nonassoc ELSE
%left SEMICOLON
%left ASSIGN
%left OR
%left AND
%nonassoc EQ NEQ GT LT GE LE
%left PLUS MINUS
%left TIMES DIVIDE
%left UMINUS
```

I am not 100% sure on how the parser will behave once these settings are made but %left PLUS MINUS
seems to instruct the parser to apply plus to the left most operator immediately when it has the
chance to. This makes the parser reduce before shift. So Bison's default conflict resolution is
overriden by the strategy to reduce first and the warning is gone.
