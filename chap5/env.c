#include "env.h"

/**
 * Creates a E_enventry object that has the kind E_varEntry.
 *
 */
E_enventry E_VarEntry(Ty_ty ty)
{
    //printf("Factory - E_VarEntry\n");

	E_enventry e = checked_malloc(sizeof(*e));
    e->kind = E_varEntry;
    e->u.var.ty = ty;

    return e;
}

/**
 * Creates a E_enventry object that has the kind E_funEntry.
 *
 */
E_enventry E_FunEntry(Ty_tyList formals, Ty_ty result)
{
    //printf("Factory - E_VarEntry\n");

	E_enventry e = checked_malloc(sizeof(*e));
    e->kind = E_funEntry;
    e->u.fun.result = result;
    e->u.fun.formals = formals;
    
    return e;
}