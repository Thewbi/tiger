#include "semant.h"

struct expty expTy(Tr_exp exp, Ty_ty ty) 
{
    struct expty e; 
    e.exp=exp; 
    e.ty=ty; 
    return e;
}

/**
 * Traverses expressions and performs semantic analysis, makes sure that the types match.
*/
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
            printf("aaa A_intExp 17\n");
            //return expTy(a, Ty_Int());
            return expTy(a, TAB_look(tenv, S_Symbol("int")));
            //break;

        case A_stringExp:
            printf("A_stringExp 18\n");
            //return expTy(a, Ty_String());
            return expTy(a, TAB_look(tenv, S_Symbol("string")));
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
            printf("A A_assignExp 23: pos: %d\n", a->pos);

            A_var var = a->u.assign.var;
            struct expty lvalue = transVar(venv, tenv, var);

            printf("B A_assignExp 23: pos: %d\n", a->pos);

            A_exp exp = a->u.assign.exp;
            struct expty rhs_exp = transExp(venv, tenv, exp);

            printf("lvalue: %d rhs_exp: %d\n", lvalue, rhs_exp);
            printf("lvalue.ty: %d rhs_exp.ty: %d\n", lvalue.ty, rhs_exp.ty);

            printf("C A_assignExp 23: pos: %d\n", a->pos);

            if (lvalue.ty->kind != rhs_exp.ty->kind)
            {
                EM_error(a->u.op.left->pos, "Types used in assignment are incompatible!");
                return expTy(NULL, Ty_Nil());
            } 
            else 
            {
                printf("A_assignExp 23 - Semantically sane! \n");
            }

            printf("D A_assignExp 23: pos: %d\n", a->pos);
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
        {
            printf("A_arrayExp 29: pos: %d\n", a->pos);

            // S_symbol typ; A_exp size, init;
            // struct {S_symbol typ; A_exp size, init;} array;
            S_symbol typ = a->u.array.typ;
            A_exp size = a->u.array.size;
            A_exp init = a->u.array.init;

            printf("typ: \"%s\"\n", S_name(typ));
            printf("size: %d\n", size->u.intt);
            printf("init: kind: %d\n", init->kind);

            printf("AAA\n");

            //struct expty typ_exp_type = transExp(venv, tenv, typ);
            //E_enventry typ_env_entry = TAB_look(tenv, typ);
            //printf("typ_env_entry: kind: %d\n", typ_env_entry);
            //printf("typ_env_entry: kind: %d\n", typ_env_entry->kind);
            //Ty_ty array_type = typ_env_entry->u.var.ty;

            printf("\nTAB_DUMP tenv\n=============================\n");
            TAB_dump(tenv, show);
            printf("=============================\n");

            //void* binding_value = TAB_look(tenv, typ);
            void* binding_value = S_look(tenv, typ);
            //void* binding_value = S_look(venv, typ);
            assert(binding_value);
            printf("binding_value: %d\n", binding_value);

            Ty_ty array_typetttt = (Ty_ty) binding_value;
            show_type(array_typetttt);

            Ty_ty array_element_type = array_typetttt->u.array;
            show_type(array_element_type);

            //printf("binding_value: %s\n", S_name(array_typetttt));

            printf("BBB\n");

            struct expty init_exp_type = transExp(venv, tenv, init);
            show_type(init_exp_type.ty);

            printf("CCC\n");

            if (init_exp_type.ty != array_element_type) {
                EM_error(a->pos, "Type used in array initialization does not match array type!");
                return expTy(NULL, Ty_Nil());
            } else {
                printf("Array declaration valid\n");
            }

            // check if the init kind matches the typ of the array
            //asdf

            //struct expty exp = expTy(a, Ty_Array(typ_env_entry->u.var));
            //return exp;

            //assert(0);

            //return expTy(a, Ty_Array(array_typetttt));
            //printf("ZINK %d\n", init_exp_type.ty);
            return expTy(a, Ty_Array(array_element_type));
            //return array_typetttt;
        }
        break;

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
 * 
 * has to return the type or a variable usage.
 * Is used in transExp().
 */
struct expty transVar(S_table venv, S_table tenv, A_var v)
{
    printf("transVar\n");

    switch(v->kind) 
    {
        case A_simpleVar:
            printf("simpleVar 12 - VarName: \"%s\"\n", S_name(v->u.simple));

            printf("\nTAB_DUMP venv\n=============================\n");
            TAB_dump(venv, show);
            printf("=============================\n");

            Ty_ty ty = TAB_look(venv, v->u.simple);
            show(v->u.simple, ty);

            if (ty == NULL) {
                EM_error(v->pos, "Variable \"%s\" is not declared. The type is unknown! Line: %d\n", S_name(v->u.simple), v->pos);
                return expTy(NULL, Ty_Nil());
            }
            return expTy(NULL, ty);

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
 * - type declarations (array, records)
 * - function declarations
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
                printf("init_type retrieved: %d\n", init_type);
                show_type(init_type.ty);
            }
            else 
            {
                printf("No initializer\n");
            }

            //printf("kind: %d\n", d->kind);

            // the type specifier on a variable is optional
            S_symbol type_symbol = d->u.var.typ;
            if (type_symbol == NULL)
            {
                S_enter(venv, d->u.var.var, init_type.ty);

                printf("\nTAB_DUMP venv\n=============================\n");
                TAB_dump(venv, show);
                printf("=============================\n");
            }
            else
            {
                printf("type_symbol: \"%s\"\n", S_name(type_symbol));

                Ty_ty var_type = S_look(tenv, type_symbol);
                show_type(var_type);

                if ((init_type.ty != Ty_Nil()) && (var_type != Ty_Nil())) {

                    printf("AAAAAA\n");

                    show_type(var_type);
                    show_type(init_type.ty);

                    printf("%d - %d\n", var_type, init_type.ty);

                    //if (var_type != init_type.ty)
                    if (var_type->u.array != init_type.ty->u.array)
                    //if (var_type->u.array != init_type.ty)
                    {

                        printf("BBBBBB\n");


                        EM_error(d->pos, "Types used in variable declaration initializer are incompatible!");
                        return expTy(NULL, Ty_Nil());
                    }
                }
                // enter binding for the declared variable into the variable environment (venv)
                // The initializer type is used because the explizit type specifier is optional and
                // the initializer and specifier types have to match at all times
                //S_enter(venv, d->u.var.var, E_VarEntry(init_type.ty));
                S_enter(venv, d->u.var.var, init_type.ty);

                // DEBUG
                printf("\nTAB_DUMP venv\n=============================\n");
                TAB_dump(venv, show);
                printf("=============================\n");
            }
        }
        break;

        case A_typeDec:
        {
            printf("A Adding to tenv: \"%s\"\n", S_name(d->u.type->head->name));
            S_enter(tenv, d->u.type->head->name, transTy(tenv, d->u.type));

            // DEBUG
            printf("\nTAB_DUMP tenv\n=============================\n");
            TAB_dump(tenv, show);
            printf("=============================\n");
        }
        break;

        default:
            assert(0);
    }

    printf("transDec done.\n");
}

/**
 * This function takes types and either looks them up in the type environment
 * or creates types
*/
/*struct*/ Ty_ty transTy(S_table tenv, A_ty a)
{
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
                void *retrieved_type = TAB_look(tenv, ty->u.name);
                
                // printf("A Adding to tenv: \"%s\"\n", S_name(named_type->name));
                // S_enter(tenv, named_type->name, retrieved_type);

                // printf("\nTAB_DUMP tenv\n=============================\n");
                // TAB_dump(tenv, show);
                // printf("=============================\n");

                return retrieved_type;
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
                Ty_ty rec_ty = Ty_Record(named_type->ty->u.record);

                // // insert this type and it's fields into tenv
                // //S_enter(tenv, named_type->name, E_VarEntry(rec));
                // printf("B Adding to tenv: \"%s\"\n", S_name(named_type->name));
                // S_enter(tenv, named_type->name, rec_ty);

                return rec_ty;
            }
            break;

            case A_arrayTy: //=35
            {
                printf("A_arrayTy - 35 - value: \"%s\"\n", S_name(ty->u.name));

                // retrieve the referenced type from the tenv
                void * retrieved_type = TAB_look(tenv, ty->u.name);

                //A_ty t = A_ArrayTy(a->pos, ty->u.name);

                printf("hohohoh\n");
                show_type(retrieved_type);

                Ty_ty array_ty = Ty_Array(retrieved_type);

                // printf("C Adding to tenv: \"%s\"\n", S_name(named_type->name));
                // //S_enter(tenv, named_type->name, E_VarEntry(retrieved_type));
                // //S_enter(tenv, named_type->name, E_VarEntry(array));
                // S_enter(tenv, named_type->name, array_ty);

                // // check
                // void* binding_value = S_look(tenv, named_type->name);
                // assert(binding_value);

                // printf("\nTAB_DUMP tenv\n=============================\n");
                // TAB_dump(tenv, show);
                // printf("=============================\n");
                
                return array_ty;
            }
            break;
        }

        // advance iterator
        named_types_list = named_types_list->tail;
    }
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

    show_type(value);

    // E_enventry env_entry = (E_enventry)value;
    // if (E_varEntry == env_entry->kind)
    // {
    //     Ty_ty type = env_entry->u.var.ty;
    //     show_type(type);
    // }
    // else
    // {
    //     show_type(env_entry);
    //     //printf("Unknown kind: %d:\n", env_entry->kind);
    // }
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