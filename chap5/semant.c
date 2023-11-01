#include "semant.h"

struct expty expTy(Tr_exp exp, Ty_ty ty) 
{
    struct expty e; 
    e.exp=exp; 
    e.ty=ty; 
    return e;
}

struct expty transExp(S_table venv, S_table tenv, A_exp a)
{
    switch(a->kind) 
    {
        case A_varExp:
            printf("A_varExp 15\n");
            return transVar(venv, tenv, a->u.var);

        case A_nilExp:
            printf("A_varExp 16\n");
            return expTy(a, Ty_Nil());
            //break;

        case A_intExp:
            printf("A_intExp 17\n");
            return expTy(a, Ty_Int());
            //break;

        case A_stringExp:
            printf("A_stringExp 18\n");
            return expTy(a, Ty_String());
            //break;

        case A_callExp:
            printf("A_callExp 19\n");
            assert(0);
            //break;

        case A_opExp: 
        {
            printf("A_opExp 20\n");

            A_oper oper = a->u.op.oper;
            struct expty left = transExp(venv, tenv, a->u.op.left);
            struct expty right = transExp(venv, tenv, a->u.op.right);
            if (oper == A_plusOp) 
            {
                printf("A_opExp 20 - PLUS \n");

                bool sane = TRUE;

                if (left.ty->kind != Ty_int)
                {
                    EM_error(a->u.op.left->pos, "integer required");
                    sane = FALSE;
                }
            
                if (right.ty->kind != Ty_int)
                {
                    EM_error(a->u.op.right->pos,"integer required");
                    sane = FALSE;
                }

                if (sane) {
                    printf("A_opExp 20 - PLUS - Semantically sane! \n");
                }
            
                return expTy(NULL, Ty_Int());
            }
        }

        case A_recordExp:
            printf("A_recordExp 21\n");
            assert(0);
            //break;

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
            struct expty lvalue = transVar(venv, tenv, var);
            // if (lvalue == NULL)
            // {
            //     printf("NULL!!!!\n");
            // }

            A_exp exp = a->u.assign.exp;
            struct expty rhs_exp = transExp(venv, tenv, exp);

            if (lvalue.ty->kind != rhs_exp.ty->kind)
            {
                EM_error(a->u.op.left->pos, "Types used in assignment are incompatible!");
                return expTy(NULL, Ty_Nil());
            } 
            else 
            {
                printf("A_assignExp 23 - Semantically sane! \n");
            }
            break;

        case A_ifExp:
            printf("A_ifExp 24\n");
            assert(0);
            //break;

	    case A_whileExp:
            printf("A_whileExp 25\n");
            assert(0);
            //break;

        case A_forExp:
            printf("A_forExp 26\n");
            assert(0);
            //break;

        case A_breakExp:
            printf("A_breakExp 27\n");
            assert(0);
            //break;

        case A_letExp: 
        {
            printf("A_letExp 28: pos: %d\n", a->pos);

            S_beginScope(venv);
            S_beginScope(tenv);

            A_decList decs = a->u.let.decs;
            while (decs != NULL) {
                printf("dec: pos: %d\n", a->pos);
                transDec(venv, tenv, decs->head);
                decs = decs->tail;
            }

            struct expty exp;
            A_exp body = a->u.let.body;
            if (body != NULL) {
                printf("body:\n");
                exp = transExp(venv, tenv, body);
            }

            S_endScope(tenv);
            S_endScope(venv);

            return exp;
        }
        break;

        case A_arrayExp:
            printf("A_arrayExp 29: pos: %d\n", a->pos);
            assert(0);
            //break;

        default:
            printf("Unknown expression! kind: %d pos: %d\n", a->kind, a->pos);
            /* should have returned from some clause of the switch */
            assert(0);
            break;
    }
}

/**
 * transVar - (Trans)late (Var)iables
 * 
 * Processes
 * - Variable usages
 */
struct expty transVar(S_table venv, S_table tenv, A_var v)
{
    printf("transVar\n");

    switch(v->kind) 
    {
        case A_simpleVar:
            printf("simpleVar 12 - VarName: \"%s\"\n", S_name(v->u.simple));
            E_enventry env_entry = TAB_look(venv, v->u.simple);
            if (env_entry == NULL) {
                EM_error(v->pos, "Variable \"%s\" is not declared. The type is unknown! Line: %d\n", S_name(v->u.simple), v->pos);
                return expTy(NULL, Ty_Nil());
            }
            return expTy(NULL, env_entry->u.var.ty);

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

/**
 * transDec - (Trans)late (Dec)larations
 * 
 * Processes
 * - variable declarations (e.g. var a:int := 2)
 * - array type declarations
 */
void transDec(S_table venv, S_table tenv, A_dec d)
{
    printf("transDec: pos: %d\n", d->pos);
    printf("kind: %d\n", d->kind);

    switch (d->kind)
    {
        case A_varDec:
        {
            // see VARIABLE DECLARATIONS, page 119
            struct expty init_type = expTy(NULL, Ty_Nil());
            // determine the type of the initialization value
            if (d->u.var.init != NULL) {
                init_type = transExp(venv, tenv, d->u.var.init);
            }
            else 
            {
                printf("No initializer\n");
            }
            Ty_ty var_type = d->u.var.typ;
            show_type(var_type);
            if ((init_type.ty != Ty_Nil()) && (var_type != NULL) && (var_type != init_type.ty)) {
                EM_error(d->pos, "Types used in variable declaration initializer are incompatible!");
                return expTy(NULL, Ty_Nil());
            }
            // enter binding for the declared variable into the variable environment (venv)
            // The initializer type is used because the explizit type specifier is optional and
            // the initializer and specifier types have to match at all times
            S_enter(venv, d->u.var.var, E_VarEntry(init_type.ty));
            TAB_dump(venv, show);
        }
        break;

        case A_typeDec:
        {
            //S_enter(tenv, d->u.type->head->name, transTy(tenv, d->u.type->head->ty));
            S_enter(tenv, d->u.type->head->name, transTy(tenv, d->u.type));
        }
        break;

        default:
            assert(0);
    }

    printf("transDec done.\n");
}

/*struct*/ Ty_ty transTy(S_table tenv, A_ty a)
{
    //A_nametyList named_types_list = d->u.type;
    A_nametyList named_types_list = a;
    while (named_types_list != NULL) {

        A_namety named_type = named_types_list->head;
        A_ty ty = named_type->ty;

        switch (ty->kind)
        {
            case A_nameTy: //=33,
            {
                printf("A_nameTy - 33 - value: \"%s\"\n", S_name(ty->u.name));
                // retrieve the referenced type from the tenv
                void * retrieved_type = TAB_look(tenv, ty->u.name);
                S_enter(tenv, named_type->name, E_VarEntry(retrieved_type));
                TAB_dump(tenv, show);
            }
            break;

            case A_recordTy: //=34,
            {
                printf("A_recordTy - 34 - named_type - value: \"%s\"\n", S_name(named_type->name));
                
                // DEBUG output field list
                A_fieldList field_list = named_type->ty->u.record;
                while (field_list != NULL) 
                {
                    A_namety record_field = field_list->head;

                    printf("name: %s type: %s\n", S_name(record_field->name), S_name(record_field->ty));

                    // advance iterator
                    field_list = field_list->tail;
                }

                // ... turning A_recordTy into Ty_Record ...
                Ty_ty rec = Ty_Record(named_type->ty->u.record);

                // insert this type and it's fields into tenv
                S_enter(tenv, named_type->name, E_VarEntry(rec));
            }
            break;

            case A_arrayTy: //=35
            {
                printf("A_arrayTy - 35 - value: \"%s\"\n", S_name(ty->u.name));
                // retrieve the referenced type from the tenv
                void * retrieved_type = TAB_look(tenv, ty->u.name);
                S_enter(tenv, named_type->name, E_VarEntry(retrieved_type));
                TAB_dump(tenv, show);
            }
            break;
        }

        // advance iterator
        named_types_list = named_types_list->tail;
    }

    //assert(0);
    //return Ty_Nil();
}

/**
 * Called by TAB_dump() in table.c/h
 * Outputs a binding stored inside a table environment
 * 
 * @param: key is a symbol that can be printed with S_name(key) which converts the symbol into a string
 * @param: value E_VarEntry
 */ 
void show(void *key, void *value)
{
    //
    // markers on in the environments require special treatment since they have null values for value
    //

    S_symbol temp = (S_symbol) key;
    if (strcmp(S_name(temp), "<mark>") == 0) {
        printf("<mark>\n");
        return;
    }

    //
    // print normal entries
    //

    printf("Key: '%s' ", S_name(key));

    E_enventry env_entry = (E_enventry)value;
    if (E_varEntry == env_entry->kind)
    {
        Ty_ty type = env_entry->u.var.ty;
        show_type(type);
    }
    else
    {
        show_type(env_entry);
        //printf("Unknown kind: %d:\n", env_entry->kind);
    }
}

void show_type(Ty_ty type) {
    if (type == NULL)
    {
        printf("No type!\n");
        return;
    }
    switch (type->kind)
    {
        case Ty_record:
            printf("Type: Ty_record\n");
            break;

        case Ty_nil:
            printf("Type: Ty_nil\n");
            break;
            
        case Ty_int:
            printf("Type: Ty_int\n");
            break;
            
        case Ty_string:
            printf("Type: Ty_string\n");
            break;
            
        case Ty_array:
            printf("Type: Ty_array\n");
            break;
            
        case Ty_name:
            printf("Type: Ty_name\n");
            break;
            
        case Ty_void:
            printf("Type: Ty_void\n");
            break;
            
        default:
            printf("Type: Unknown type!\n");
            break;
    }
} 