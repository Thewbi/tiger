%{
#include <string.h>
#include "util.h"
#include "tokens.h"
#include "errormsg.h"

int charPos=1;

int yywrap(void)
{
 charPos=1;
 return 1;
}

void adjust(void)
{
 EM_tokPos=charPos;
 charPos+=yyleng;
}

%}
/* lex definitions */

digits [0-9]+

%option nounput
%option noinput

/* https://gist.github.com/DmitrySoshnikov/f5e2583b37e8f758c789cea9dcdf238a */
/* A lexer rule can optionally have one or more start conditions. E.g.: <INITIAL>if {adjust(); return IF;} */
/* The lexer is initially in the 0 aka. INITIAL condition, where only the rules without any start condition are active */
/* 0 and INITIAL are synonyms. To return to the default INITIAL condition, execute BEGIN 0 or BEGIN INITIAL (brackets are optional) */
/* To change the lexer condition, use the BEGIN(<CONDITION_NAME_HERE>) function. */
/* The brackets of the BEGIN function are optional. BEGIN INITIAL is also valid.
/* When changing the lexer conditions, the lexer will change the rules it will to scan input. */
/* It will only execute the rules that match the lexer's current condition */
/* In a sense, conditions are use to put the lexer into different modes. For example the lexer could */
/* go into special condition for comments where most of the rules are deactivated and only the comment rules are active */
/* */
/* To define new conditions, %s and %x are used followed by a list of new conditions */
/* %s defines inclusive start conditions. Rules with the start condition and also rules without start condition are active. */
/* %x defines exclusive start conditions. Only rules with the start condition are active! */

%x COMMENT STRING_STATE

%%

  /* reserved words */
array {adjust(); return ARRAY;}
if   {adjust(); return IF;}
then {adjust(); return THEN;}
else {adjust(); return ELSE;}
while {adjust(); return WHILE;}
for  	 {adjust(); return FOR;}
to  {adjust(); return TO;}
do  {adjust(); return DO;}
let  {adjust(); return LET;}
in   {adjust(); return IN;}
end   {adjust(); return END;}
of   {adjust(); return OF;}
break   {adjust(); return BREAK;}
nil   {adjust(); return NIL;}
function   {adjust(); return FUNCTION;}
var   {adjust(); return VAR;}
type   {adjust(); return TYPE;}

  /*punctuations*/
":" {adjust(); return COLON;}
";" {adjust(); return SEMICOLON;}
"(" {adjust(); return LPAREN;}
")" {adjust(); return RPAREN;}
"[" {adjust(); return LBRACK;}
"]" {adjust(); return RBRACK;}
"{" {adjust(); return LBRACE;}
"}" {adjust(); return RBRACE;}
"." {adjust(); return DOT;}
"+" {adjust(); return PLUS;}
"-" {adjust(); return MINUS;}
"*" {adjust(); return TIMES;}
"/" {adjust(); return DIVIDE;}
"=" {adjust(); return EQ;}
"<>" {adjust(); return NEQ;}
"<" {adjust(); return LT;}
"<=" {adjust(); return LE;}
">" {adjust(); return GT;}
">=" {adjust(); return GE;}
"&" {adjust(); return AND;}
"|" {adjust(); return OR;}
":=" {adjust(); return ASSIGN;}



[a-zA-Z][a-zA-Z0-9]* {adjust();yylval.sval=String(yytext); return ID;}


" "	 {adjust(); continue;}
\n	 {adjust(); EM_newline(); continue;}
\t	 {adjust(); continue;}
","	 {adjust(); return COMMA;}

{digits}	 {adjust(); yylval.ival=atoi(yytext); return INT;}
.	 {adjust(); EM_error(EM_tokPos,"illegal token");}



  /* comment handling */

  /* When in INITIAL condition and a comment starts, go to COMMENT condition */
<INITIAL>"/*" {adjust(); BEGIN COMMENT;}

  /* When in COMMENT condition and a comment starts, go back to INITIAL condition */
<COMMENT>"*/" {adjust(); BEGIN INITIAL;}

  /* increment line numbers while inside multilien comments also */
<COMMENT>\n	 {adjust(); EM_newline(); continue;}

  /* When in COMMENT condition, consume all characters and ignore them */
<COMMENT>. {adjust();}

  /* Detect unclosed comments on end of file EOF */
<COMMENT><<EOF>>     {adjust(); EM_error(EM_tokPos,"unclosed comment detected! %s", yytext);  yyterminate();}


