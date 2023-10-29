#include "semant.h"

struct expty expTy(Tr_exp exp, Ty_ty ty) 
{
    struct expty e; 
    e.exp=exp; 
    e.ty=ty; 
    return e;
}

//struct expty transVar(S_table venv, S_table tenv, A_var v);

struct expty transExp(S_table venv, S_table tenv, A_exp a)
{
    switch(a->kind) 
    {
        case A_opExp: 
        {
            A_oper oper = a->u.op.oper;
            struct expty left = transExp(venv, tenv, a->u.op.left);
            struct expty right = transExp(venv, tenv, a->u.op.right);
            if (oper == A_plusOp) 
            {
                if (left.ty->kind!=Ty_int)
                {
                    EM_error(a->u.op.left->pos, "integer required");
                }
            
                if (right.ty->kind!=Ty_int)
                {
                    EM_error(a->u.op.right->pos,"integer required");
                }
            
                return expTy(NULL, Ty_Int());
            }
        }

        default:
            printf("Unknown expression! kind: %d\n", a->kind);
            break;
    }

    /* should have returned from some clause of the switch */
    assert(0);
}

//void transDec(S_table venv, S_table tenv, A_dec d);
//struct Ty_ty transTy ( S_table tenv, A_ty a);