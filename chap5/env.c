#include "env.h"

/**
 * Creates a E_enventry object that has the kind E_varEntry.
 *
 */
E_enventry E_VarEntry(Ty_ty ty)
{
    printf("Factory - E_VarEntry\n");
	E_enventry e = checked_malloc(sizeof(*e));
    e->kind = E_varEntry;
    e->u.var.ty = ty;
    return e;
}