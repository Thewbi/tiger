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

    printf("semanttest.c - A\n");

    A_exp program = parse(argv[1]);

    printf("semanttest.c - B\n");

    // open a file in write only mode
    FILE *out_file = fopen("ast_dump.txt", "w"); 
    
    // test for files not existing.
    if (out_file == NULL)
    {  
        printf("Error! Could not open file\n");

        // must include stdlib.h
        exit(-1); 
    }

    printf("semanttest.c - D\n");

    printf("Dumping the AST ...\n");
    pr_exp(out_file, program, 0);
    printf("Dumping the AST done.\n");

    printf("semanttest.c - E\n");

    printf("Performing Semantic Analysis ...\n");
    S_table venv;
    S_table tenv;
    transExp(venv, tenv, program);
    printf("Performing Semantic Analysis done.\n");

    // close the file
    fclose(out_file);
    out_file = NULL;

    return 0;
}
