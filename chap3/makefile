LEXER_GENERATOR = flex
PARSER_GENERATOR = bison
CC=gcc

a.out: parsetest.o y.tab.o lex.yy.o errormsg.o util.o
	$(CC) -o parsetest -g parsetest.o y.tab.o lex.yy.o errormsg.o util.o

parsetest.o: parsetest.c errormsg.h util.h
	$(CC) -g -c parsetest.c

y.tab.o: y.tab.c
	$(CC) -g -c y.tab.c

y.tab.c: tiger.grm
	$(PARSER_GENERATOR) -dv tiger.grm -o y.tab.c

y.tab.h: y.tab.c
	echo "y.tab.h was created at the same time as y.tab.c"

errormsg.o: errormsg.c errormsg.h util.h
	$(CC) -g -c errormsg.c

lex.yy.o: lex.yy.c y.tab.h errormsg.h util.h
	$(CC) -g -c lex.yy.c

lex.yy.c: tiger.lex
	$(LEXER_GENERATOR) tiger.lex

util.o: util.c util.h
	$(CC) -g -c util.c

clean: 
	rm -f a.out parsetest.exe util.o parsetest.o lex.yy.o errormsg.o y.tab.c y.tab.h y.tab.o y.output tiger.output lex.yy.c tiger.tab.grm tiger.tab.h tiger.tab.hh
