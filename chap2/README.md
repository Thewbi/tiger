# lexical-analysis
lexical analysis for tiger in c++

## Installing the tools

Under Windows, to get access to a C compiler, install cygwin (https://www.cygwin.com/) and the dev packages.
Downloading cygwin will give you a setup-x86_64.exe which is a download manager for all packages that cygwin provides.
Use it to install the dev packages. Add C:\cygwin64\bin to your system environment variables Path variable.

Install GNU make for windows (https://gnuwin32.sourceforge.net/packages/make.htm)
Add the folder C:\Program Files (x86)\GnuWin32\bin to your system environment variables Path variable.

As an editor I use VSCode https://code.visualstudio.com/. 
I starts up and closes down really quickly and is generally lightweight enough to not be a burden.

## Running the tests

To test your lexer, type make to build the lextest.exe file.
Make will process the provided Makefile and build the executable.
Once the lextest.exe is available, you can run the lexer on the sample files.

```
lextest.exe ..\testcases\test6.tig
```

The lexer has to list the tokens without any error messages.
If there are error messages, you have to work on the .lex file and build using make again until the lexer
processes all the samples.

## Work on the lexer

A lexer chops the input stream into token. The token go into the parser which is not a concern of chapter 2.
To build a lexer, lex (or gnu flex) is used.

lex will process .lex files and output generated C code. This C code is you lexer that you can compile into
your compiler to give it a lexer. Since there is no compiler application yet, a small C application is 
provided that runs the generated C code. This program is called a driver. The driver is compiled into
the lextest.exe. lextest.exe has one command line argument which is the .tig file to lex.

Back to the .lex file. The initial .lex file prodivded by Andrew W. Apple is a template to start from.
It is not capable of parsing tiger. Several changes have been made (and copied from other compiler writers)
to make this lexer work.

### Lexer states

The general lexer functionality is that characters or character sequences are defined and for each of these
sequence a rule is defined. The lexer identifies the character sequences and the executes the attached actions.

A layer on top of these rules is constructed using states aka. conditions.
The lexer is initially in the INITIAL or 0 condition/state.

New states can be defined by the user. The lexer can be told to change states within rules.
To change state, the BEGIN keyword followed by the new state is used.
Going back to the initial state is achieved by using BEGIN INITIAL or BEGIN 0.
Braces are optional BEGIN(0) is also valid.

A rule can be optionally assigned to one or more states.
That means that a state unlocks certain sets of rules. The lexer will only perform a subset of all rules
when it is in a specific state.

This is usefull if you encounter comments or strings. Withing a comment or a string, the lexer should not
emit any token except a String or a comment token! So the rules for all token have to be disabled.
This is achieved using states for INITIAL, where all token are emitted, COMMENT, where only comments are
lexed and STRING_STATE where Strings are emitted.

### Activating states / modes

It is important to place the rules that change mode above the rule for any character (.)
Otherwhise the rules to change state will not be activated ever.

```
/* lexer state/mode/condition transitions from INITIAL to states handling comments and strings */

  /* When in INITIAL condition and a comment starts, go to COMMENT condition */
<INITIAL>"/*" {adjust(); commentNesting++; BEGIN COMMENT;}

  /* When in INITIAL condition and a string starts, go to STRING_STATE condition */
<INITIAL>\" {adjust(); init_str_buf(); BEGIN STRING_STATE; }
```

### Parsing comments

Two exclusive lexer states are defined in the template but not implemented:

```
%x COMMENT STRING_STATE
```

%x means the state is exclusive, this means only rules that explicitly have the state as a condition are executed.
Alternatively there are also %s states which execute the rules that explicitly list the condition and also 
all rules that have no condition specified.

In the comment state the following rules apply:

```
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
```

Basically the return to INITIAL happens when the */ token is lexed and when no deep comment nesting (commentNesting == 0)
currently takes place.

When a comment starts within a comment, the comment nesting is incremented (commentNesting++). The lexer no has to
ascend from as many nested comments as have been started before.

Newlines are processed. When the end of file is reached (<<EOF>>) whilst inside a comment, the lexer stops outputting
an error message.

Generally all characters (.) are consumend but also ignored i.e. they are not stored inside any buffer.


# Parsing strings

String lexing is organized very similar to comment lexing.

In the string state the following rules apply:

```
<STRING_STATE>{

  \" {adjust(); BEGIN INITIAL; yylval.sval = strdup(str_buf); return STRING; }

  <<EOF>>	 {adjust(); EM_error(EM_tokPos,"Unclosed string detected! %s", yytext);  yyterminate();}

  . {
      adjust();
      char *yptr=yytext;
      append_char2str_buf(*yptr);
  }

}
```

<<EOF>> within a string leads to an error.

All characters (.) are consumed but this time they are also copied into a buffer (append_char2str_buf()).
The function that initializes the buffer is called on entering the STRING_STATE. The function implementations
for the initilize and the add function are placed into the .lex file! lex/flex will take the C code
and paste them into the generated C code so that all the functions you place into the .lex file actually
end up in the generated lexer source code!

When the closing quote is seen, the lexer goes back to INITIAL. It also returns a STRING token.
The token has the char buffer attached to the yylval.sval value.

### Parsing identifiers

The underscore has been added to identifiers in order to be able to parse sample test6.tig which
contains the identifier do_nothing1.

```
[a-zA-Z][_a-zA-Z0-9]* {adjust();yylval.sval=String(yytext); return ID;}
```
 
