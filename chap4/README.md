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

The variable $$ is the result of the rule. When a non-terminal is reduced, it is reduced to a single
symbol. That symbol is the symbol on the left side of the rule. This symbol caries a value with it.
The $$ symbol is used to assign a value to the reduced left hand side symbol. When that symbol occurs
in a rule on the right-hand side, and that rule is reduced, the semantic action has access to the value
assigned to the symbol using $1, $2, $3, ....

The variables $1, $2, $3, ... denote the individual symbols on the right-hand side when a non-terminal 
is reduced. You have to realize that when a non-terminal is reduced and the right-hand side contains
non-terminals then these non-terminals have been reduced before hence they already have a value assigned
to them. You can then take the values from the non-terminals and combine them in new ways into a value
that you then assign to the $$ variable to return it to other semantic actions that are executed 
later.

There is on important thing to realize when using the variables $1, $2, $3, ... as explained in this
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

tkFOR is $1, tkIDENT is $2, the semantic action after tkIDENT is $3, tkEQUALS is $4, expr is $5, the 
semantic action after expr is $6, ...

Imagine the non-terminal rule:

```
let : LET decs IN expseq END
```

$$ is the value that will be stored inside let.

$1 is LET
$2 is the value that is already stored inside decs.
$3 is IN
$4 is the value that is already stored inside expseq.
$5 is the value ...




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




## Error: error: $$ of 'nil' has no declared type

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

Some of the rules will use $$ and $$ is defined to be the int pos.

```
%type <pos> exp
```

Some of the rules will use $$ and $$ is defined to be the int ival.

```
%type <ival> exp2
```

Some of the rules will use $$ and $$ is defined to be the int sval.

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

You have added a new field to the union. No bison complains that it does not
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



## Using the symbols

The file absyn.h contains functions that construct nodes that can be inserted into the AST.

