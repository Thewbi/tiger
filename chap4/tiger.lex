%{
#include <string.h>
#include "util.h"
#include "absyn.h"

#include "y.tab.h"
#include "errormsg.h"


/*
 * Variable to keep track of the depth that comments are nested.
 */
int commentNesting = 0;

/*
 * strings defines
 */
const int INITIAL_BUF_LEN = 32;
char *str_buf;
unsigned int str_buf_cap;

/*
 * Initialize the string buffer.
 */
void init_str_buf(void){
    str_buf = checked_malloc(INITIAL_BUF_LEN);
    // 0 stands for end in a char array
    str_buf[0] = 0;
    str_buf_cap = INITIAL_BUF_LEN;
}

/*
 * Append the given character to the string buffer and double
 * the buffer's capacity if necessary.
 */
static void append_char2str_buf(char ch){
    size_t new_length = strlen(str_buf) + 1;
    if (new_length == str_buf_cap){
        char *temp;
        str_buf_cap *= 2;
        temp = checked_malloc(str_buf_cap);
        memcpy(temp, str_buf, new_length);
        free(str_buf);
        str_buf = temp;
    }
    str_buf[new_length - 1] = ch;
    str_buf[new_length] = 0;
}

/*
 * Variable to keep track of the position of each token, measured in characters
 * since the beginning of the file.
 */
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



[a-zA-Z][_a-zA-Z0-9]* {adjust();yylval.sval=String(yytext); return ID;}




  /* lexer state/mode/condition transitions from INITIAL to states handling comments and strings */

  /* When in INITIAL condition and a comment starts, go to COMMENT condition */
<INITIAL>"/*" {adjust(); commentNesting++; BEGIN COMMENT;}

  /* When in INITIAL condition and a string starts, go to STRING_STATE condition */
<INITIAL>\" {adjust(); init_str_buf(); BEGIN STRING_STATE; }






" "	 {adjust(); continue;}
\r?\n	 {adjust(); EM_newline(); continue;}
\t	 {adjust(); continue;}
","	 {adjust(); return COMMA;}

{digits}	 {adjust(); yylval.ival=atoi(yytext); return INT;}
.	 {adjust(); EM_error(EM_tokPos,"illegal token");}



  


<STRING_STATE>{

  \" {adjust(); BEGIN INITIAL; yylval.sval = strdup(str_buf); return STRING; }

  <<EOF>>	 {adjust(); EM_error(EM_tokPos,"Unclosed string detected! %s", yytext);  yyterminate();}

  . {
      adjust();
      char *yptr=yytext;
      append_char2str_buf(*yptr);
  }

}
  
<COMMENT>{
  
    /* When in COMMENT condition and a comment ends, go back to INITIAL condition */
  "*/" {adjust(); commentNesting--; if (commentNesting == 0) { BEGIN INITIAL; } }

    /* When in COMMENT condition and a comment starts, stay in COMMENT condition and increment the comment nesting */
  "/*" {adjust(); commentNesting++;}

    /* increment line numbers while inside multilien comments also */
  \n	 {adjust(); EM_newline(); continue;}

    /* Detect unclosed comments on end of file EOF */
  <<EOF>>     {adjust(); EM_error(EM_tokPos,"Unclosed comment detected! %s", yytext);  yyterminate();}

    /* When in COMMENT condition, consume all characters and ignore them */
  . {adjust();}

}


