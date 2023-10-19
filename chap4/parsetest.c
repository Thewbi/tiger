#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "errormsg.h"
#include "parse.h"
#include "prabsyn.h"

//FILE* fout = NULL;

extern int yyparse(void);


/*
void parse(string fname) 
{EM_reset(fname);
 if (yyparse() == 0) 
   fprintf(stderr,"Parsing successful!\n");
 else fprintf(stderr,"Parsing failed\n");
}*/


int main(int argc, char **argv) {

  

 if (argc!=2) {fprintf(stderr,"usage: a.out filename\n"); exit(1);}

 printf("A\n");

A_exp program = parse(argv[1]);

printf("B\n");

//  if (!fout) {
//   printf("C\n");
 
//       fout = stdout;
//       }

FILE *out_file = fopen("ast_dump.txt", "w"); // write only
// test for files not existing.
          if (out_file == NULL)
            {  
              printf("Error! Could not open file\n");
              exit(-1); // must include stdlib.h
            }

 printf("D\n");

 pr_exp(out_file, program, 0);

printf("E\n");

fclose(out_file);
out_file = NULL;

 return 0;
}
