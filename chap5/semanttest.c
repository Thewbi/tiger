#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "errormsg.h"
#include "parse.h"
#include "prabsyn.h"

#include "semant.h"

extern int yyparse(void);

int main(int argc, char **argv) 
{
    if (argc!=2) 
    {
        fprintf(stderr,"usage: a.out filename\n"); exit(1);
    }

    printf("\n\n\nParsing the input ...\n");
    A_exp program = parse(argv[1]);
    printf("Parsing the input done.\n");

    if (program == NULL)
    {
        printf("No root exp retrieved! Aborting!\n");
        return 0;
    }

    printf("\n\n\nDumping the AST ...\n");
    // open a file in write only mode
    FILE *out_file = fopen("ast_dump.txt", "w"); 
    // test for files not existing.
    if (out_file == NULL)
    {  
        printf("Error! Could not open file\n");
        // must include stdlib.h
        exit(-1); 
    }
    pr_exp(out_file, program, 0);
    // close the file
    fclose(out_file);
    out_file = NULL;
    printf("Dumping the AST done.\n");

    



    //
    // initial type environment
    //

    S_table tenv = S_empty();

    // insert the built in types into the type environment (tenv)
    S_enter(tenv, S_Symbol("int"), Ty_Int());
    S_enter(tenv, S_Symbol("string"), Ty_String());

    // DEBUG
    printf("\nINITIAL TAB_DUMP venv\n=============================\n");
    TAB_dump(tenv, show);
    printf("=============================\n");



    //
    // initial variable environment
    //

    S_table venv = S_empty();

    // The base_venv environment contains bindings for predefined functions
    // flush, ord, chr, size, and so on, described in Appendix A
    //
    // Several functions are predefined:

    // function print(s : string)
    // Print s on standard output.

    // function flush()
    // Flush the standard output buffer.

    // function getchar() : string
    // Read a character from standard input; return empty string on end of file.
    Ty_tyList getchar_formals = NULL;
    Ty_ty getchar_result_ty = Ty_String();
    S_enter(venv, S_Symbol("getchar"), E_FunEntry(getchar_formals, getchar_result_ty));

    // function ord(s: string) : int
    // Give ASCII value of first character of s; yields -1 if s is empty string.
    A_field ord_first_formal_param = A_Field(0, S_Symbol("s"), S_Symbol("string"));
    A_fieldList ord_params = A_FieldList(ord_first_formal_param, NULL);
    Ty_tyList ord_formals = makeFormalTyList(tenv, ord_params);
    Ty_ty ord_result_ty = Ty_Int();
    S_enter(venv, S_Symbol("ord"), E_FunEntry(ord_formals, ord_result_ty));

    // function chr(i: int) : string
    // Single-character string from ASCII value i; halt program if i out of range.

    // function size(s: string) : int
    // Number of characters in s.

    // function substring(s:string, first:int, n:int) : string
    // Substring of string s, starting with character first, n characters long. Characters are numbered starting at 0.

    // function concat (s1: string, s2: string) : string
    // Concatenation of s1 and s2.

    // function not(i : integer) : integer
    // Return (i=0).

    // function exit(i: int)
    // Terminate execution with code i

    // DEBUG
    printf("\nINITIAL TAB_DUMP venv\n=============================\n");
    TAB_dump(venv, show);
    printf("=============================\n");






    printf("\n\n\nPerforming Semantic Analysis ...\n");
    transExp(venv, tenv, program);
    printf("Performing Semantic Analysis done.\n");

    return 0;
}
