# Building the AST

The abstract syntax tree (AST) describes the symbols and their hierarchical structure as parsed from the input.
In chapter 4, the symbols defined by Mr. Apple are used to construct the AST.
Also there is a function provided that prints the AST to a FILE object. This function is used to print
the AST to a .txt file.

## Constructing the AST using Bison

The strategy to construct the AST was taken from https://cop3402fall20.github.io/projects/project1.html and
from the files that Mr. Apple provides.

Bison has semantic actions that are small snippets of C code that you can assign to non-terminals aka.
productions in the grammar. A semantic action is enclosed in squiggly brackets.

```
exp :   NUM     {$$ = $1;}
    | ID        {$$ = vars[$1];}
    | exp '+' exp   {$$ = $1 + $3;}
    | ID '=' exp    {$$ = vars[$1] = $3;}
;
```

Your C snippets will be copied directly into the generated parser. They are called whenever
the non-terminal is reduced.

Within the snippets, there are several Bison specific variables avaliable that the parser will fill
with certain specific values. Those variables are the interface between the snippets and the 
parser if you want.

The variable \$\$ is the result of the rule. When a non-terminal is reduced, it is reduced to a single
symbol. That symbol is the symbol on the left side of the rule. This symbol caries a value with it.
The $$ symbol is used to assign a value to the reduced left hand side symbol. When that symbol occurs
in a rule on the right-hand side, and that rule is reduced, the semantic action has access to the value
assigned to the symbol using \$1, \$2, \$3, ....

The variables \$1, \$2, \$3 and so on denote the individual symbols on the right-hand side when a non-terminal 
is reduced. You have to realize that when a non-terminal is reduced and the right-hand side contains
non-terminals then these non-terminals have been reduced before hence they already have a value assigned
to them. You can then take the values from the non-terminals and combine them in new ways into a value
that you then assign to the \$\$ variable to return it to other semantic actions that are executed 
later.

There is on important thing to realize when using the variables \$1, \$2, \$3, ... as explained in this
stack overflow question: https://stackoverflow.com/questions/43882160/bison-grammar-type-and-token
The important point is that you semantic actions are actually counted along the symbols on the right
hand side. If you insert a semantic action, this action will use up one of the indexes and all the
following symbols and semantic actions are pushed back and use the following indexes.

As an example:

```
for_statement : tkFOR 
                tkIDENT   { printf( "I:%s\n", $2 );  }
                tkEQUALS 
                expr      { printf( "A:%d\n", $5 /* $<nVal>5 */ );    }
                tkTO 
                expr      { printf( "A:%d\n", $8 /* $<nVal>8 */ );    }
                step-statement 
                list 
                next-statement;
```

tkFOR is \$1, tkIDENT is $2, the semantic action after tkIDENT is \$3, tkEQUALS is \$4, expr is \$5, the 
semantic action after expr is \$6, ...

Imagine the non-terminal rule:

```
let : LET decs IN expseq END
```

\$\$ is the value that will be stored inside let.

\$1 is LET
\$2 is the value that is already stored inside decs.
\$3 is IN
\$4 is the value that is already stored inside expseq.
\$5 is the value ...




## Error:  error: rule given for vardec, which is a token

The definition

```
%token <dec> vardec
```

causes the problem. It defines vardec to be a token, and later
in the same grammer, there is also a non-terminal rule that has vardec
as the left-hand side.

This means you have to either delete the token/terminal or the non-terminal/rule.
Otherwise the symbol is used twice for different things which causes the error.




## Error: error: \$\$ of 'nil' has no declared type

Your types are not set up correctly. Here is how types work in Bison.

The union defines which types exist:

```
%union {
	int pos;
	int ival;
	string sval;
	}
```

E.g. there is a integer called pos, a integer called ival and a String called sval.

Next you have to select for each rule, which of the fields in the union is used.
This selected field will become $$ for that rule!

Some of the rules will use \$\$ and \$\$ is defined to be the int pos.

```
%type <pos> exp
```

Some of the rules will use \$\$ and \$\$ is defined to be the int ival.

```
%type <ival> exp2
```

Some of the rules will use \$\$ and \$\$ is defined to be the string sval.

```
%type <sval> exp3
```

Each rule can only use at most one field of the union!

The generated code in y.tab.c will look something like this:

```
  case 23: /* nil: NIL  */
#line 124 "tiger.grm"
          { (yyval.a_exp) = A_NilExp(0); }
#line 1374 "y.tab.c"
    break;
```


## Error:  unknown type name 'A_dec'

You have added a new field to the union. Now bison complains that it does not
know the type!

As stated here: https://stackoverflow.com/questions/40028904/yacc-union-on-struct-type

The error occurs when parsing a file that includes the generated parser.
This file is missing the include for the type used inside the union.
You have to find and fix this file.

The solution was to include #include "absyn.h" before #include "y.tab.h" in tiger.lex:

```
#include <string.h>
#include "util.h"
#include "absyn.h"

#include "y.tab.h"
#include "errormsg.h"
```

The reason why this worked is that tiger.lex is converted to C code by the flex
code generator. The generated C code had no include for the types it used. Adding
an include into the .lex file causes this include to get generated into the final C code and
the C code now can resolve the missing types.



# Using the symbols

The file absyn.h contains functions that construct nodes that can be inserted into the AST.




# Approach Developing and Testing the Software for Chapter 4

Constructing and printing the AST means to have semantic actions for all parts of the grammar to construct the AST.

The most efficient way to work towards completion is to start with the most basic production rules and work your
way up towards the higher level productions that make use of the low level production rules.

As an example, one of the most basic rules is to have an expression that consists of the integer literal 1 only.
1 is a valid expression hence it is already a valid tiger program and hence a AST can be constructed for it and
printed onto the console or into a text file.

Starting from the expression 1, you can now construct operation usage such as 1+2 and logical operator usage
such as 1 and 2. From there construct variable usage such as 1+i or 1+k or 1 + variable_1.

Then make sure assignments like a := 1 can be parsed and converted to an AST.

Then go to sequences. 

Once sequences are in place, you should quickly get let statements right.

Proceed with if-then and if-then-else.

Then go to while and for loops.

Then go to type declaractions or function declarations.

In terms of the testcases provided in the book, the proposed order of testing is:

Atomic Expressions:
vardec_no_type_and_initializer.tig          ( parsetest.exe ..\testcases\vardec_no_type_and_initializer.tig & cat ast_dump.txt )

Assignments:
assignment.tig      ( parsetest.exe ..\testcases\assignment.tig & cat ast_dump.txt )

Sequences:
sequencing.tig      ( parsetest.exe ..\testcases\sequencing.tig & cat ast_dump.txt )

Let
let.tig             ( parsetest.exe ..\testcases\let.tig & cat ast_dump.txt )
let_nested.tig      ( parsetest.exe ..\testcases\let_nested.tig & cat ..\testcases\let_nested.tig & cat ast_dump.txt )

All the following tests are in the testcases\book folder

Comparison Operators:
test13.tig          ( parsetest.exe ..\testcases\book\test13.tig & cat ast_dump.txt )

Arithmetic Operators:
test26.tig          ( parsetest.exe ..\testcases\book\test26.tig & cat ..\testcases\book\test26.tig & cat ast_dump.txt )

if-then-else:
test8.tig,          ( parsetest.exe ..\testcases\book\test8.tig & cat ast_dump.txt )
test9.tig,          ( parsetest.exe ..\testcases\book\test9.tig & cat ..\testcases\book\test9.tig & cat ast_dump.txt )
test15.tig          ( parsetest.exe ..\testcases\book\test15.tig & cat ..\testcases\book\test15.tig & cat ast_dump.txt )

while-loops:
test10.tig          ( parsetest.exe ..\testcases\book\test10.tig & cat ..\testcases\book\test10.tig & cat ast_dump.txt )

for-loops:
test11.tig          ( parsetest.exe ..\testcases\book\test11.tig & cat ..\testcases\book\test11.tig & cat ast_dump.txt )

sequences:
test20.tig          ( parsetest.exe ..\testcases\book\test20.tig & cat ..\testcases\book\test20.tig & cat ast_dump.txt )

array-indexing:
test24.tig          ( parsetest.exe ..\testcases\book\test24.tig & cat ..\testcases\book\test24.tig & cat ast_dump.txt )
test32.tig          ( parsetest.exe ..\testcases\book\test32.tig & cat ..\testcases\book\test32.tig & cat ast_dump.txt )

record field acces:
test25.tig          ( parsetest.exe ..\testcases\book\test25.tig & cat ..\testcases\book\test25.tig & cat ast_dump.txt )

variable declarations:
test12.tig,         ( parsetest.exe ..\testcases\book\test12.tig & cat ..\testcases\book\test12.tig & cat ast_dump.txt )
test31.tig,         ( parsetest.exe ..\testcases\book\test31.tig & cat ..\testcases\book\test31.tig & cat ast_dump.txt )
test37.tig,         ( parsetest.exe ..\testcases\book\test37.tig & cat ..\testcases\book\test37.tig & cat ast_dump.txt )
test41.tig,         ( parsetest.exe ..\testcases\book\test41.tig & cat ..\testcases\book\test41.tig & cat ast_dump.txt )
test42.tig,         ( parsetest.exe ..\testcases\book\test42.tig & cat ..\testcases\book\test42.tig & cat ast_dump.txt )
test43.tig          ( parsetest.exe ..\testcases\book\test43.tig & cat ..\testcases\book\test43.tig & cat ast_dump.txt )

record type definitions:
test33.tig,         ( parsetest.exe ..\testcases\book\test33.tig & cat ..\testcases\book\test33.tig & cat ast_dump.txt )
test44.tig,         ( parsetest.exe ..\testcases\book\test44.tig & cat ..\testcases\book\test44.tig & cat ast_dump.txt )
test45.tig,         ( parsetest.exe ..\testcases\book\test45.tig & cat ..\testcases\book\test45.tig & cat ast_dump.txt )
test46.tig,         ( parsetest.exe ..\testcases\book\test46.tig & cat ..\testcases\book\test46.tig & cat ast_dump.txt )
test47.tig,         ( parsetest.exe ..\testcases\book\test47.tig & cat ..\testcases\book\test47.tig & cat ast_dump.txt )
test49.tig   OK BECAUSE OF SYNTAX ERROR ( parsetest.exe ..\testcases\book\test49.tig & cat ..\testcases\book\test49.tig & cat ast_dump.txt )

type definitions:
test16.tig,         ( parsetest.exe ..\testcases\book\test16.tig & cat ..\testcases\book\test16.tig & cat ast_dump.txt )
test1.tig,          ( parsetest.exe ..\testcases\book\test1.tig & cat ..\testcases\book\test1.tig & cat ast_dump.txt )
test2.tig,          ( parsetest.exe ..\testcases\book\test2.tig & cat ..\testcases\book\test2.tig & cat ast_dump.txt )
test3.tig,          ( parsetest.exe ..\testcases\book\test3.tig & cat ..\testcases\book\test3.tig & cat ast_dump.txt )
test5.tig,          ( parsetest.exe ..\testcases\book\test5.tig & cat ..\testcases\book\test5.tig & cat ast_dump.txt )
test14.tig,         ( parsetest.exe ..\testcases\book\test14.tig & cat ..\testcases\book\test14.tig & cat ast_dump.txt )
test17.tig,         ( parsetest.exe ..\testcases\book\test17.tig & cat ..\testcases\book\test17.tig & cat ast_dump.txt )
test22.tig,         ( parsetest.exe ..\testcases\book\test22.tig & cat ..\testcases\book\test22.tig & cat ast_dump.txt )
test23.tig,         ( parsetest.exe ..\testcases\book\test23.tig & cat ..\testcases\book\test23.tig & cat ast_dump.txt )
test28.tig,         ( parsetest.exe ..\testcases\book\test28.tig & cat ..\testcases\book\test28.tig & cat ast_dump.txt )
test29.tig,         ( parsetest.exe ..\testcases\book\test29.tig & cat ..\testcases\book\test29.tig & cat ast_dump.txt )
test30.tig,         ( parsetest.exe ..\testcases\book\test30.tig & cat ..\testcases\book\test30.tig & cat ast_dump.txt )
test38.tig,         ( parsetest.exe ..\testcases\book\test38.tig & cat ..\testcases\book\test38.tig & cat ast_dump.txt )
test48.tig          ( parsetest.exe ..\testcases\book\test48.tig & cat ..\testcases\book\test48.tig & cat ast_dump.txt )

function declarations and function calls:
test4.tig,          ( parsetest.exe ..\testcases\book\test4.tig & cat ..\testcases\book\test4.tig & cat ast_dump.txt )
test6.tig,          ( parsetest.exe ..\testcases\book\test6.tig & cat ..\testcases\book\test6.tig & cat ast_dump.txt )
test7.tig,          ( parsetest.exe ..\testcases\book\test7.tig & cat ..\testcases\book\test7.tig & cat ast_dump.txt )
test18.tig,         ( parsetest.exe ..\testcases\book\test18.tig & cat ..\testcases\book\test18.tig & cat ast_dump.txt )
test19.tig,         ( parsetest.exe ..\testcases\book\test19.tig & cat ..\testcases\book\test19.tig & cat ast_dump.txt )
test34.tig,         ( parsetest.exe ..\testcases\book\test34.tig & cat ..\testcases\book\test34.tig & cat ast_dump.txt )
test35.tig,         ( parsetest.exe ..\testcases\book\test35.tig & cat ..\testcases\book\test35.tig & cat ast_dump.txt )
test36.tig,         ( parsetest.exe ..\testcases\book\test36.tig & cat ..\testcases\book\test36.tig & cat ast_dump.txt )
test39.tig,         ( parsetest.exe ..\testcases\book\test39.tig & cat ..\testcases\book\test39.tig & cat ast_dump.txt )
test40.tig          ( parsetest.exe ..\testcases\book\test40.tig & cat ..\testcases\book\test40.tig & cat ast_dump.txt )

function calls:
test21.tig,         ( parsetest.exe ..\testcases\book\test21.tig & cat ..\testcases\book\test21.tig & cat ast_dump.txt )
test27.tig          ( parsetest.exe ..\testcases\book\test27.tig & cat ..\testcases\book\test27.tig & cat ast_dump.txt )

parsetest.exe ..\testcases\BartVandewoestyne\compilable\array_equality.tig
parsetest.exe ..\testcases\BartVandewoestyne\compilable\control_characters.tig
parsetest.exe ..\testcases\BartVandewoestyne\compilable\escape_sequences.tig
parsetest.exe ..\testcases\BartVandewoestyne\compilable\function.tig
parsetest.exe ..\testcases\BartVandewoestyne\compilable\negative_int.tig
parsetest.exe ..\testcases\BartVandewoestyne\compilable\procedure.tig
parsetest.exe ..\testcases\BartVandewoestyne\compilable\record_equality_test.tig
parsetest.exe ..\testcases\BartVandewoestyne\compilable\simple_let.tig
parsetest.exe ..\testcases\BartVandewoestyne\compilable\valid_strings.tig


# Implementing Sequences

Extend the expseq rule in your tiger.grm by semantic actions and use the functions from absyn.h
to construct nodes that will then form the AST. absyn.h is already provided in the book's source
code (https://www.cs.princeton.edu/~appel/modern/c/project.html). The task at hand is to call the
correct provided function at the right time. Management of nodes it taken care of already.

As noted in the book, for academic purposes, memory is only ever allocated but there are no
free calls to return the memory. This is not a problem since the application will not run for 
an extended period of time say on a server. Therefore when the application terminates the memory
is returned to the operating system immediately. But keep in mind that for production grade software 
(server software or short running application, open source or payed-for closed source) this approach of 
not returning memory would not be acceptable!

```
expseq :
       exp { $$ = A_ExpList($1, NULL); }
       | 
       exp SEMICOLON expseq { $$ = A_ExpList($1, $3); }
       ;
```
Both semantic actions will add a node to the single linked list of expressions in the sequence.
The rule for exp is adding a NULL pointer denoting that there is no next node to follow.
The semicolon rule does construct a node and a pointer to the rest of the list of expressions
which extends the single linked list by one new expression.

Do not forget to extend the exp and the sequencing rules! They have to pass on the constructed nodes.

```
exp : 

    ...

    | sequencing { $$ = $1; }

    ...

```

exp will just forward the node it sees without any further processing.

```
sequencing : LPAREN expseq RPAREN { $$ = A_SeqExp(0, $2); }
           ;
```

The sequenceing rule will not only pass on a node, but it also constructs a A_SeqExp node
wrapping the nodes it received from it's predecessors further down the AST.
