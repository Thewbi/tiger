#ifndef _SEMANT_H_
#define _SEMANT_H_

#include <stdio.h>
#include <stdlib.h>

#include "symbol.h"
#include "types.h"
#include "absyn.h"
#include "env.h"
#include "table.h"

#define OUTSIDE_LOOP 0
#define INSIDE_LOOP 1

// where Tr_exp is the translation of the expression into intermediate code, and
// ty is the type of the expression.
// To avoid a discussion of intermediate code at this point, let us define a
// dummy Translate module:
typedef void *Tr_exp;

// The result will be an expty, containing a translated
// expression and its Tiger-language type:
struct expty {Tr_exp exp; Ty_ty ty;};
struct expty expTy(Tr_exp exp, Ty_ty ty);

struct expty transVar(S_table venv, S_table tenv, A_var v);
struct expty transExp(S_table venv, S_table tenv, A_exp a, int inside_loop);
void transDec(S_table venv, S_table tenv, A_dec d);
/*struct*/ Ty_ty transTy (S_table tenv, A_ty a);

Ty_tyList makeFormalTyList(S_table tenv, A_fieldList params);

void show(void *key, void *value);
void show_type(Ty_ty type);
char* label_type(Ty_ty type);

#endif