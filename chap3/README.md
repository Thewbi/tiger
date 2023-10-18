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
(This file is included into the lexer (tiger.lex) so that the lexer generator lex/flex knows which
ids to use for the token it emits)
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