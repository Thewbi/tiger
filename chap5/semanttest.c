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

    printf("\n\n\nPerforming Semantic Analysis ...\n");
    S_table venv = S_empty();
    S_table tenv = S_empty();

    // insert the built in types into the type environment (tenv)
    S_enter(tenv, S_Symbol("int"), Ty_Int());
    S_enter(tenv, S_Symbol("string"), Ty_String());

    transExp(venv, tenv, program);
    printf("Performing Semantic Analysis done.\n");

    return 0;
}
