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
    A_field print_first_formal_param = A_Field(0, S_Symbol("s"), S_Symbol("string"));
    A_fieldList print_params = A_FieldList(print_first_formal_param, NULL);
    Ty_tyList print_formals = makeFormalTyList(tenv, print_params);
    Ty_ty print_result_ty = NULL;
    S_enter(venv, S_Symbol("print"), E_FunEntry(print_formals, print_result_ty));

    // function flush()
    // Flush the standard output buffer.
    Ty_tyList flush_formals = NULL;
    Ty_ty flush_result_ty = NULL;
    S_enter(venv, S_Symbol("flush"), E_FunEntry(flush_formals, flush_result_ty));

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
    A_field chr_first_formal_param = A_Field(0, S_Symbol("i"), S_Symbol("int"));
    A_fieldList chr_params = A_FieldList(chr_first_formal_param, NULL);
    Ty_tyList chr_formals = makeFormalTyList(tenv, chr_params);
    Ty_ty chr_result_ty = Ty_Int();
    S_enter(venv, S_Symbol("chr"), E_FunEntry(chr_formals, chr_result_ty));

    // function size(s: string) : int
    // Number of characters in s.
    A_field size_first_formal_param = A_Field(0, S_Symbol("s"), S_Symbol("string"));
    A_fieldList size_params = A_FieldList(size_first_formal_param, NULL);
    Ty_tyList size_formals = makeFormalTyList(tenv, size_params);
    Ty_ty size_result_ty = Ty_Int();
    S_enter(venv, S_Symbol("size"), E_FunEntry(size_formals, size_result_ty));

    // function substring(s:string, first:int, n:int) : string
    // Substring of string s, starting with character first, n characters long. Characters are numbered starting at 0.
    A_field substring_first_formal_param = A_Field(0, S_Symbol("s"), S_Symbol("string"));
    A_field substring_second_formal_param = A_Field(0, S_Symbol("first"), S_Symbol("int"));
    A_field substring_third_formal_param = A_Field(0, S_Symbol("n"), S_Symbol("int"));
    A_fieldList substring_params = A_FieldList(substring_first_formal_param, NULL);
    substring_params = A_FieldList(substring_second_formal_param, substring_params);
    substring_params = A_FieldList(substring_third_formal_param, substring_params);
    Ty_tyList substring_formals = makeFormalTyList(tenv, substring_params);
    Ty_ty substring_result_ty = Ty_String();
    S_enter(venv, S_Symbol("substring"), E_FunEntry(substring_formals, substring_result_ty));

    // function concat(s1: string, s2: string) : string
    // Concatenation of s1 and s2.
    A_field concat_first_formal_param = A_Field(0, S_Symbol("s1"), S_Symbol("string"));
    A_field concat_second_formal_param = A_Field(0, S_Symbol("s2"), S_Symbol("string"));
    A_fieldList concat_params = A_FieldList(concat_first_formal_param, NULL);
    concat_params = A_FieldList(concat_second_formal_param, concat_params);
    Ty_tyList concat_formals = makeFormalTyList(tenv, concat_params);
    Ty_ty concat_result_ty = Ty_String();
    S_enter(venv, S_Symbol("concat"), E_FunEntry(concat_formals, concat_result_ty));

    // function not(i : integer) : integer
    // Return (i=0).
    A_field not_first_formal_param = A_Field(0, S_Symbol("i"), S_Symbol("int"));
    A_fieldList not_params = A_FieldList(not_first_formal_param, NULL);
    Ty_tyList not_formals = makeFormalTyList(tenv, not_params);
    Ty_ty not_result_ty = Ty_Int();
    S_enter(venv, S_Symbol("not"), E_FunEntry(not_formals, not_result_ty));

    // function exit(i: int)
    // Terminate execution with code i
    A_field exit_first_formal_param = A_Field(0, S_Symbol("i"), S_Symbol("int"));
    A_fieldList exit_params = A_FieldList(exit_first_formal_param, NULL);
    Ty_tyList exit_formals = makeFormalTyList(tenv, exit_params);
    Ty_ty exit_result_ty = NULL;
    S_enter(venv, S_Symbol("exit"), E_FunEntry(exit_formals, exit_result_ty));

    // DEBUG
    printf("\nINITIAL TAB_DUMP venv\n=============================\n");
    TAB_dump(venv, show);
    printf("=============================\n");






    printf("\n\n\nPerforming Semantic Analysis ...\n");
    transExp(venv, tenv, program);
    printf("Performing Semantic Analysis done.\n");

    return 0;
}
