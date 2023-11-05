#ifndef _ENV_H_
#define _ENV_H_

#include "symbol.h"
#include "types.h"

typedef struct E_enventry_ *E_enventry;

struct E_enventry_ {
    enum {E_varEntry = 254, E_funEntry = 255} kind;
    union {
        struct { Ty_ty ty; } var;
        struct { Ty_tyList formals; Ty_ty result; } fun;
    } u;
};

E_enventry E_VarEntry(Ty_ty ty);
E_enventry E_FunEntry(Ty_tyList formals, Ty_ty result);
S_table E_base_tenv(void); /* Ty_ ty environment */
S_table E_base_venv(void); /* E_ enventry environment */

#endif