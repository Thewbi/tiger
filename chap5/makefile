LEXER_GENERATOR = flex
PARSER_GENERATOR = bison
CC=gcc

semanttest.exe: semanttest.o y.tab.o lex.yy.o errormsg.o util.o absyn.o prabsyn.o symbol.o parse.o table.o types.o semant.o env.o
	$(CC) -o semanttest.exe -g semanttest.o y.tab.o lex.yy.o errormsg.o util.o absyn.o prabsyn.o symbol.o parse.o table.o types.o semant.o env.o

semanttest.o: semanttest.c errormsg.h util.h
	$(CC) -g -c semanttest.c

table.o: table.c
	$(CC) -g -c table.c

parse.o: parse.c
	$(CC) -g -c parse.c

symbol.o: symbol.c
	$(CC) -g -c symbol.c

prabsyn.o: prabsyn.c
	$(CC) -g -c prabsyn.c

absyn.o: absyn.c
	$(CC) -g -c absyn.c

util.o: util.c util.h
	$(CC) -g -c util.c

types.o: types.c types.h
	$(CC) -g -c types.c

semant.o: semant.c semant.h
	$(CC) -g -c semant.c

env.o: env.c env.h
	$(CC) -g -c env.c

errormsg.o: errormsg.c errormsg.h util.h
	$(CC) -g -c errormsg.c

y.tab.o: y.tab.c
	$(CC) -g -c y.tab.c

y.tab.c: tiger.grm
	$(PARSER_GENERATOR) -dv tiger.grm -o y.tab.c

y.tab.h: y.tab.c
	echo "y.tab.h was created at the same time as y.tab.c"

lex.yy.o: lex.yy.c y.tab.h errormsg.h util.h
	$(CC) -g -c lex.yy.c

lex.yy.c: tiger.lex
	$(LEXER_GENERATOR) tiger.lex

clean: 
	rm -f semanttest.exe semanttest.exe.stackdump a.exe util.o semanttest.o lex.yy.o lex.yy.c errormsg.o y.tab.c y.tab.h y.tab.o tiger.output tiger.tab.grm ast_dump.txt symbol.o absyn.o prabsyn.o table.o y.output parse.o semant.o types.o env.o
