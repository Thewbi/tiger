/*
 * parse.c - Parse source file.
 */

#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "absyn.h"
#include "errormsg.h"
#include "parse.h"

#include "y.tab.h"

extern int yyparse(void);

// this symbol is defined in tiger.grm
extern A_exp absyn_root;

/* parse source file fname; 
   return abstract syntax data structure */
A_exp parse(string fname) 
{
  EM_reset(fname);

  if (yyparse() == 0) {

    fprintf(stderr,"Parsing successful!\n");
  
    /* parsing worked */
    return absyn_root;
  }
  else
  {
    fprintf(stderr,"Parsing failed\n");
    return NULL;
  }
}
