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

        case A_intExp:
            printf("aaa A_intExp 17\n");
            return expTy(a, TAB_look(tenv, S_Symbol("int")));

        case A_stringExp:
            printf("A_stringExp 18\n");
            return expTy(a, TAB_look(tenv, S_Symbol("string")));

        case A_callExp:
            printf("A_callExp 19\n");
            assert(0);

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
        break;

        case A_recordExp:
        {
            printf("A_recordExp 21\n");

            S_symbol typ = a->u.record.typ;
            printf("A_recordExp - typ: \"%s\"\n", S_name(typ));

            // lookup the symbol in tenv
            void* binding_value = S_look(tenv, typ);
            assert(binding_value);
            printf("binding_value: %d\n", binding_value);

            Ty_ty record_type = (Ty_ty) binding_value;
            show_type(record_type);

            return expTy(a, record_type);
        }
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
        {
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
        }
        break;

        case A_ifExp:
            printf("A_ifExp 24\n");
            assert(0);

	    case A_whileExp:
            printf("A_whileExp 25\n");
            assert(0);

        case A_forExp:
            printf("A_forExp 26\n");
            assert(0);

        case A_breakExp:
            printf("A_breakExp 27\n");
            assert(0);

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

            printf("\nTAB_DUMP tenv\n=============================\n");
            TAB_dump(tenv, show);
            printf("=============================\n");

            void* binding_value = S_look(tenv, typ);
            assert(binding_value);
            printf("binding_value: %d\n", binding_value);

            Ty_ty array_type = (Ty_ty) binding_value;
            show_type(array_type);

            Ty_ty array_element_type = array_type->u.array;
            show_type(array_element_type);

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
            //return expTy(a, array_element_type);

            return expTy(a, array_type);
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
        {
            printf("A_fieldVar 13 - VarName: \"%s\"\n", S_name(v->u.field.sym));

            // struct {A_var var; S_symbol sym;} field;

            A_var record_variable = v->u.field.var;
            printf("Kind: %d\n", record_variable->kind);

            S_symbol record_variable_name = record_variable->u.simple;
            printf("??22: %s\n", S_name(record_variable_name));

            // convert variable name to it's type
            Ty_ty var_type = S_look(venv, record_variable_name);
            show_type(var_type);

            // DEBUG
            printf("\nTAB_DUMP venv\n=============================\n");
            TAB_dump(venv, show);
            printf("=============================\n");

            // DEBUG
            printf("\nTAB_DUMP tenv\n=============================\n");
            TAB_dump(tenv, show);
            printf("=============================\n");

            A_fieldList field_list = var_type->u.record;
            while (field_list != NULL) 
            {
                A_namety record_field = field_list->head;
                printf("name: %s type: %s\n", S_name(record_field->name), S_name(record_field->ty));

                if (v->u.field.sym == record_field->name) {
                    return expTy(v, TAB_look(tenv, record_field->ty));
                }

                // advance iterator
                field_list = field_list->tail;
            }

            assert(0);
        }

        case A_subscriptVar:
            printf("A_subscriptVar 14 - VarName: \"%s\"\n", S_name(v->u.subscript.var->u.simple));

            // this is the name of the array type:
            printf("Array Variable Name: %s\n", S_name(v->u.subscript.var->u.simple));

            Ty_ty array_type = TAB_look(venv, v->u.subscript.var->u.simple);
            show_type(array_type);

            // subscripts are only allowd on Ty_array
            if (array_type->kind != Ty_array)
            {
                EM_error(v->pos, "Subscript used on variable \"%s\". Variable is not an array! Line: %d\n", S_name(v->u.subscript.var->u.simple), v->pos);
            }
            
            Ty_ty array_element_type = array_type->u.array;
            show_type(array_element_type);

            return expTy(NULL, array_element_type);

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

                printf("d->u.var.init - kind: %d\n", d->u.var.init->kind);

                init_type = transExp(venv, tenv, d->u.var.init);

                printf("init_type retrieved: %d\n", init_type);
                show_type(init_type.ty);
            }
            else 
            {
                printf("No initializer\n");
            }

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
                printf("Current tenv\n");

                // DEBUG
                printf("\nTAB_DUMP tenv\n=============================\n");
                TAB_dump(tenv, show);
                printf("=============================\n");

                printf("type_symbol: \"%s\"\n", S_name(type_symbol));

                Ty_ty var_type = S_look(tenv, type_symbol);
                show_type(var_type);

                if ((init_type.ty != Ty_Nil()) && (var_type != Ty_Nil())) {

                    printf("AAAAAA\n");

                    show_type(var_type);
                    show_type(init_type.ty);

                    printf("%d - %d\n", var_type, init_type.ty);

                    switch (var_type->kind) {

                        case Ty_array:
                            if (var_type->u.array != init_type.ty->u.array)
                            {
                                printf("BBBBBB\n");

                                EM_error(d->pos, "Types used in variable declaration initializer are incompatible!");
                                return expTy(NULL, Ty_Nil());
                            }
                            break;

                        case Ty_record:
                            if (var_type->u.record != init_type.ty->u.record)
                            {
                                printf("CCCCCCCC\n");

                                EM_error(d->pos, "Types used in variable declaration initializer are incompatible!");
                                return expTy(NULL, Ty_Nil());
                            }
                            break;

                        default:
                            printf("Unknown var type: %d\n", var_type->kind);
                            assert(0);
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

        case A_functionDec:
        {
            printf("A_functionDec\n");

            A_fundecList fundecList = d->u.function;

            while (fundecList != NULL) 
            {
                A_fundec fundec = fundecList->head;
                printf("fundec: pos: %d\n", fundec->pos);
                printf("fundec: name: %s\n", S_name(fundec->name));

                Ty_ty resultTy = S_look(tenv, fundec->result);
                Ty_tyList formalTys = makeFormalTyList(tenv, fundec->params);

                // ???
                S_enter(venv, fundec->name, E_FunEntry(formalTys, resultTy));

                // start parameter scope
                S_beginScope(venv);

                {
                    A_fieldList l;
                    Ty_tyList t;
                    for (l = fundec->params, t = formalTys; l; l = l->tail, t = t->tail)
                    {
                        printf("param: %s\n", S_name(l->head->name));
                        S_enter(venv, l->head->name, E_VarEntry(t->head));
                    }
                }

                // DEBUG
                printf("\n123 TAB_DUMP venv\n=============================\n");
                TAB_dump(venv, show);
                printf("=============================\n");

                // process the body
                transExp(venv, tenv, fundec->body);

                // remove parameter scope
                S_endScope(venv);

                fundecList = fundecList->tail;
            }
        }
        break;

        default:
            assert(0);
    }

    printf("transDec done.\n");
}

/**
 * First, transDec looks up the result-type identifier "rt" in the type environment. 
 * Then it calls the local function makeFormalTyList(), which traverses the list of 
 * formal parameters and returns a list of their types (by looking each
 * parameter type-id in the tenv).
 */
Ty_tyList makeFormalTyList(S_table tenv, A_fieldList params)
{
    printf("makeFormalTyList() \n");

    Ty_tyList result = NULL;

    while (params != NULL)
    {
        A_field param = params->head;
        printf("param: %s\n", S_name(param->name));

        Ty_ty paramTy = S_look(tenv, param->typ);
        show_type(paramTy);

        result = Ty_TyList(paramTy, result);

        params = params->tail;
    }

    printf("makeFormalTyList() done.\n");

    return result;
}

/**
 * This function takes types and either looks them up in the type environment
 * or creates types
*/
/*struct*/ Ty_ty transTy(S_table tenv, A_ty a)
{
    printf("transTy\n");

    assert(a);

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
                return TAB_look(tenv, ty->u.name);
            }
            break;

            case A_recordTy: //=34,
            {
                printf("A_recordTy - 34 - named_type - value: \"%s\"\n", S_name(named_type->name));
                
                // insert a type for all record fields into the tenv
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

                return rec_ty;
            }
            break;

            case A_arrayTy: //=35
            {
                printf("A_arrayTy - 35 - value: \"%s\"\n", S_name(ty->u.name));

                // retrieve the referenced type from the tenv
                void * retrieved_type = TAB_look(tenv, ty->u.name);

                printf("hohohoh\n");
                show_type(retrieved_type);

                Ty_ty array_ty = Ty_Array(retrieved_type);
                
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
}

void show_type(Ty_ty type)
{
    if (type == NULL)
    {
        printf("No type!\n");
        return;
    }
    
    switch (type->kind)
    {
        case E_varEntry:
        {
            printf("Type: E_varEntry\n");

            E_enventry enventry = (E_enventry) type;

            show_type(enventry->u.var.ty);

            // Ty_tyList formals = enventry->u.fun.formals;
            // while (formals != NULL) {
            //     show_type(formals->head);
            //     formals = formals->tail;
            // }

        }
        break;

        case E_funEntry:
        {
            printf("Type: E_funEntry\n");

            E_enventry enventry = (E_enventry) type;

            printf("Result-");
            show_type(enventry->u.fun.result);

            int param_idx = 1;
            Ty_tyList formals = enventry->u.fun.formals;
            while (formals != NULL) {
                
                printf("Param-%d: ", param_idx);
                param_idx = param_idx + 1;
                show_type(formals->head);
                formals = formals->tail;
            }
        }
        break;

        // 0
        case Ty_record:
        {
            printf("Type: Ty_record\n");

            // DEBUG print all the records fields, their names and types
            A_fieldList field_list = type->u.record;
            while (field_list != NULL) 
            {
                A_namety record_field = field_list->head;
                printf("name: %s type: %s\n", S_name(record_field->name), S_name(record_field->ty));

                // advance iterator
                field_list = field_list->tail;
            }
        }
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