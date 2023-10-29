#include "semant.h"

struct expty expTy(Tr_exp exp, Ty_ty ty) 
{
    struct expty e; 
    e.exp=exp; 
    e.ty=ty; 
    return e;
}

struct expty transVar(S_table venv, S_table tenv, A_var v)
{
    printf("transVar\n");

    switch(v->kind) 
    {
        case A_simpleVar:
            printf("simpleVar 12 - VarName: \"%s\"\n", S_name(v->u.simple));
            break;

        case A_fieldVar:
            printf("A_fieldVar 13\n");
            break;

        case A_subscriptVar:
            printf("A_subscriptVar 14\n");
            break;

        default:
            printf("Unknown expression! kind: %d pos: %d\n", v->kind, v->pos);
            /* should have returned from some clause of the switch */
            assert(0);
            break;
    }
}

struct expty transExp(S_table venv, S_table tenv, A_exp a)
{
    switch(a->kind) 
    {
        case A_varExp:
            printf("A_varExp 15\n");
            transVar(venv, tenv, a->u.var);
            break;

        case A_nilExp:
            printf("A_varExp 16\n");
            break;

        case A_intExp:
            printf("A_intExp 17\n");
            break;

        case A_stringExp:
            printf("A_stringExp 18\n");
            break;

        case A_callExp:
            printf("A_callExp 19\n");
            break;

        case A_opExp: 
        {
            printf("A_opExp 20\n");

            A_oper oper = a->u.op.oper;
            struct expty left = transExp(venv, tenv, a->u.op.left);
            struct expty right = transExp(venv, tenv, a->u.op.right);
            if (oper == A_plusOp) 
            {
                printf("A_opExp 20 - PLUS \n");

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

        case A_recordExp:
            printf("A_recordExp 21\n");
            break;

        case A_seqExp:
            {
                printf("A_seqExp 22\n");

                A_expList seq = a->u.seq;
                while (seq != NULL) {
                    if (seq->head != NULL)
                    {
                        printf("A_seqExp:\n");
                        transExp(venv, tenv, seq->head);
                    }
                    seq = seq->tail;
                }
            }
            break;

        case A_assignExp:
            printf("A_assignExp 23: pos: %d\n", a->pos);

            //struct {A_var var; A_exp exp;} assign;

            A_var var = a->u.assign.var;
            transVar(venv, tenv, var);

            A_exp exp = a->u.assign.exp;
            transExp(venv, tenv, exp);

            break;

        case A_ifExp:
            printf("A_ifExp 24\n");
            break;

	    case A_whileExp:
            printf("A_whileExp 25\n");
            break;

        case A_forExp:
            printf("A_forExp 26\n");
            break;

        case A_breakExp:
            printf("A_breakExp 27\n");
            break;

        case A_letExp: 
        {
            printf("A_letExp 28: pos: %d\n", a->pos);

            A_decList decs = a->u.let.decs;
            while (decs != NULL) {
                printf("dec: pos: %d\n", a->pos);
                transDec(venv, tenv, decs->head);
                decs = decs->tail;
            }

            A_exp body = a->u.let.body;
            if (body != NULL) {
                printf("body:\n");
                transExp(venv, tenv, body);
            }
        }

        case A_arrayExp:
            printf("A_arrayExp 19: pos: %d\n", a->pos);
            break;

        // case A_varDec:
        // {
        //     printf("A_varDec\n");

        //     // // struct {S_symbol var; S_symbol typ; A_exp init; bool escape;} var;
        //     // S_symbol var = a->u.var.var;
        //     // S_symbol typ = a->u.var.typ;
        //     // A_exp init = a->u.var.init;
        //     // bool escape = a->u.var.escape;
        //     transVar(venv, tenv, a);
        // }

        // case A_decList:
        // {
        //     printf("A_decList\n");
        // }

        default:
            printf("Unknown expression! kind: %d pos: %d\n", a->kind, a->pos);
            /* should have returned from some clause of the switch */
            assert(0);
            break;
    }
}

void transDec(S_table venv, S_table tenv, A_dec d)
{
    printf("transDec: pos: %d\n", d->pos);
}

// struct Ty_ty transTy(S_table tenv, A_ty a)
// {
//     printf("transTy: pos: %d\n", a->pos);
//     return Ty_Void();
// }