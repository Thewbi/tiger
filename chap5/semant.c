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
struct expty transExp(S_table venv, S_table tenv, A_exp a, int inside_loop)
{
    //printf("transExp A - a->kind: %d\n", a->kind);

    switch(a->kind) 
    {
        case A_varExp:
            //printf("A_varExp 15\n");
            return transVar(venv, tenv, a->u.var);

        case A_nilExp:
            //printf("A_varExp 16\n");
            return expTy(a, Ty_Nil());
            //return expTy(a, Ty_nil);

        case A_intExp:
            //printf("aaa A_intExp 17\n");
            return expTy(a, TAB_look(tenv, S_Symbol("int")));

        case A_stringExp:
            //printf("A_stringExp 18\n");
            return expTy(a, TAB_look(tenv, S_Symbol("string")));

        case A_callExp:
        {
            //printf("A_callExp 19\n");

            // struct {S_symbol func; A_expList args;} call;
            S_symbol func = a->u.call.func;
            A_expList args = a->u.call.args;

            //printf("A_callExp A\n");

            // retrieve the function declaration
            Ty_ty func_ty = TAB_look(venv, func);

            if (func_ty == NULL)
            {
                EM_error(a->pos, "Use of undeclared function \"%s\" !\n", S_name(func));
                assert(0);
            }

            show(func, func_ty);
            //printf("\n");

            //printf("A_callExp B\n");

            E_enventry enventry = (E_enventry) func_ty;

            //printf("A_callExp C\n");

            //printf("Result-");
            show_type(enventry->u.fun.result);
            //printf("\n");

            //printf("A_callExp 19 - A\n");

            int param_idx = 1;
            Ty_tyList formals = enventry->u.fun.formals;
            while (formals != NULL) {

                Ty_ty formal_param_ty = formals->head;

                // if a actual parameter is missing, return an error
                if (args == NULL)
                {
                    EM_error(a->pos, "Missing actual parameter in function \"%s\" !\n", S_name(func));
                    assert(0);
                }

                //printf("A_callExp 19 - AAAAAA args: %d\n", args);
                // convert the expression of the current actual parameter into a type
                struct expty actual_param_expTy = transExp(venv, tenv, args->head, OUTSIDE_LOOP);
                Ty_ty actual_param_ty = actual_param_expTy.ty;
                //printf("A_callExp 19 - BBBBBBB\n");

                // DEBUG
                //printf("Formal Param-%d: ", param_idx);
                show_type(formal_param_ty);
                //printf(" Actual Param-%d: ", param_idx);
                show_type(actual_param_ty);
                //printf("\n");

                // check the types
                if (formal_param_ty->kind != actual_param_ty->kind)
                {
                    EM_error(a->pos, "Actual and formal parameter %d are of incompatible types!\n", param_idx);
                    assert(0);
                }

                param_idx = param_idx + 1;

                // advance pointers
                formals = formals->tail;
                args = args->tail;
            }

            //printf("A_callExp 19 - B\n");

            // the return value of the call is the return type of the function declaration.
            // functions do not always have to have a return type!
            if (enventry->u.fun.result == NULL)
            {
                //printf("A_callExp 19 - C\n");
                return expTy(a, Ty_Nil());
                //return expTy(a, Ty_nil);
            }

            //printf("A_callExp 19 - D - result: %d\n", enventry->u.fun.result);

            //return transExp(venv, tenv, enventry->u.fun.result); // why was this line here?

            return expTy(a, enventry->u.fun.result); // this line is here because of getchar.tig
        }
        break;

        case A_opExp: 
        {
            //printf("A_opExp 20\n");

            A_oper oper = a->u.op.oper;
            
            if (
                (oper == A_plusOp) || (oper == A_minusOp) || (oper == A_timesOp) || (oper == A_divideOp) ||
                (oper == A_ltOp) || (oper == A_leOp) || (oper == A_gtOp) || (oper == A_geOp) ||

                (oper == A_andOp) || (oper == A_orOp)
            )
            {
                //printf("A_opExp 20 - OPERAND A exp-left: %d, exp-right: %d\n", a->u.op.left, a->u.op.right);

                // Special case: Unary minus
                if ((a->u.op.left == NULL) && oper == A_minusOp) {
                    return expTy(a, Ty_Int());
                }

                //printf("A_opExp left transExp()\n");
                struct expty left = transExp(venv, tenv, a->u.op.left, inside_loop);

                //printf("A_opExp right transExp()\n");
                struct expty right = transExp(venv, tenv, a->u.op.right, inside_loop);

                //printf("A_opExp 20 - OPERAND B left: %d, right: %d, left-ty: %d, right-ty: %d\n", left, right, left.ty, right.ty);

                bool sane = TRUE;

                if (left.ty->kind != Ty_int)
                {
                    //printf("Kind: %d\n", left.ty->kind);
                    EM_error(a->u.op.left->pos, "left operand invalid - integer required");
                    assert(0);
                    sane = FALSE;
                }

                //printf("A_opExp 20 - OPERAND C \n");
            
                if (right.ty->kind != Ty_int)
                {
                    EM_error(a->u.op.right->pos, "right operand invalid - integer required");
                    assert(0);
                    sane = FALSE;
                }

                //printf("A_opExp 20 - OPERAND D \n");

                // if (sane) {
                //     //printf("A_opExp 20 - Semantically sane! \n");
                // }

                //printf("A_opExp 20 - OPERAND D \n");
            
                return expTy(a, Ty_Int());
            }
            else if 
            (
                (oper == A_eqOp) || (oper == A_neqOp)
            )
            {
                //printf("A_opExp 20 - OPERAND F \n");

                struct expty left = transExp(venv, tenv, a->u.op.left, inside_loop);
                struct expty right = transExp(venv, tenv, a->u.op.right, inside_loop);

                //printf("A_opExp 20 - OPERAND G left: %d, right: %d, left-ty: %d, right-ty: %d\n", left, right, left.ty, right.ty);

                bool sane = TRUE;

                if ((left.ty->kind != Ty_nil) && (right.ty->kind != Ty_nil) && (left.ty->kind != right.ty->kind))
                {
                    EM_error(a->u.op.right->pos, "operand types do not match!");
                    assert(0);
                    sane = FALSE;
                }

                //printf("A_opExp 20 - OPERAND H \n");

                // if (sane) {
                //     printf("A_opExp 20 - Semantically sane! \n");
                // }

                //printf("A_opExp 20 - OPERAND I \n");
            
                return expTy(a, Ty_Int()); // 0 == false, everything else is true
            }

            //printf("Unknown operator! A_oper kind: %d\n", a->u.op.oper);
            EM_error(a->pos, "Uknown operator");
            assert(0);
        }
        break;

        case A_recordExp:
        {
            //printf("A_recordExp 21\n");

            S_symbol typ = a->u.record.typ;
            //printf("A_recordExp - type: \"%s\"\n", S_name(typ));

            // lookup the symbol in tenv
            void* binding_value = S_look(tenv, typ);
            if (binding_value == NULL)
            {
                EM_error(a->pos, "undeclared type \"%s\". tenv does not contain type: \"%s\"\n", S_name(typ), S_name(typ));
                assert(0);
            }
            assert(binding_value);
            //printf("binding_value: %d\n", binding_value);

            Ty_ty record_type = (Ty_ty) binding_value;
            show_type(record_type);

            return expTy(a, record_type);
        }
        break;

        case A_seqExp:
        {
            //printf("A_seqExp 22\n");

            struct expty last_type;
            //last_type.ty = Ty_nil;
            last_type.ty = Ty_Nil();

            A_expList seq = a->u.seq;
            while (seq != NULL) {

                if (seq->head != NULL)
                {
                    //printf("A_seqExp:\n");
                    last_type = transExp(venv, tenv, seq->head, inside_loop);
                }

                // advance iterator
                seq = seq->tail;
            }

            // the sequence adopts the type of it's last expression
            //return expTy(a, last_type.ty);
            return last_type;
        }
        break;

        case A_assignExp:
        {
            //printf("A A_assignExp 23: pos: %d\n", a->pos);

            A_var var = a->u.assign.var;
            struct expty lvalue = transVar(venv, tenv, var);

            //printf("B A_assignExp 23: pos: %d\n", a->pos);

            A_exp exp = a->u.assign.exp;
            struct expty rhs_exp = transExp(venv, tenv, exp, inside_loop);

            //printf("lvalue: %d rhs_exp: %d\n", lvalue, rhs_exp);
            //printf("lvalue.ty: %d rhs_exp.ty: %d\n", lvalue.ty, rhs_exp.ty);

            //printf("C A_assignExp 23: pos: %d\n", a->pos);

            printf("lvalue.ty \n");
            show_type(lvalue.ty);
            printf("\nrhs_exp.ty \n");
            show_type(rhs_exp.ty);
            printf("\n");

            if (lvalue.ty != rhs_exp.ty)
            //if (lvalue.ty->kind != rhs_exp.ty->kind)
            {
                EM_error(a->u.op.left->pos, "Types used in assignment are incompatible!");
                assert(0);
                //return expTy(NULL, Ty_Nil());
                //return expTy(NULL, Ty_nil);
            } 
            else 
            {
                //printf("A_assignExp 23 - Semantically sane! \n");
            }

            //printf("D A_assignExp 23: pos: %d\n", a->pos);
        }
        break;

        case A_ifExp:
        {
            //printf("A_ifExp 24\n");

            // struct {A_exp test, then, elsee;} iff; /* elsee is optional */
            A_exp test = a->u.iff.test;
            A_exp then = a->u.iff.then;
            A_exp elsee = a->u.iff.elsee;

            struct expty test_Ty = transExp(venv, tenv, test, inside_loop);
            struct expty then_Ty = transExp(venv, tenv, then, inside_loop);

            if (elsee == NULL) {
                // a if-then statement (without else) has no type because the 
                // else case is undefined. A function that has if-else as it's
                // last statement is not fully defined in the else case! Therefore
                // return nil
                return expTy(a, Ty_Nil());
                //return expTy(a, Ty_nil);
            } else {
                struct expty elsee_Ty = transExp(venv, tenv, elsee, inside_loop);

                //printf("if-then type: [");
                show_type(then_Ty.ty);
                //printf("] else type: [");
                show_type(elsee_Ty.ty);
                //printf("]\n");

                if ((then_Ty.ty->kind != Ty_nil) && (elsee_Ty.ty->kind != Ty_nil) && (then_Ty.ty->kind != elsee_Ty.ty->kind))
                {
                    EM_error(a->pos, "if-then-else invalid! then and else are of different type!");
                    assert(0);
                    return expTy(NULL, Ty_Nil());
                    //return expTy(NULL, Ty_nil);
                }
            }
            return expTy(a, then_Ty.ty);
        }
        break;

	    case A_whileExp:
        {
            //printf("A_whileExp 25\n");

            //struct {A_exp test, body;} whilee;

            A_exp test = a->u.whilee.test;
            A_exp body = a->u.whilee.body;

            struct expty test_Ty = transExp(venv, tenv, test, inside_loop);

            return transExp(venv, tenv, body, INSIDE_LOOP);
        }
        break;

        case A_forExp:
        {
            //printf("A_forExp 26\n");

            // struct {S_symbol var; A_exp lo,hi,body; bool escape;} forr;

            //printf("A_forExp A\n");
            
            S_beginScope(venv);

            //printf("A_forExp B\n");

            S_symbol var = a->u.forr.var;
            A_exp lo = a->u.forr.lo;
            A_exp hi = a->u.forr.hi;
            A_exp body = a->u.forr.body;

            //printf("A_forExp C\n");

            // DEBUG
            //printf("A_forExp 26 - var %s\n", S_name(var));

            //printf("A_forExp D\n");



            // insert iterator variable with type int into the for-loops venv
            S_enter(venv, var, Ty_Int());

            //printf("A_forExp E\n");


            // lo has to be int
            struct expty lo_ty = transExp(venv, tenv, lo, inside_loop);
            if (lo_ty.ty->kind != Ty_int)
            {
                S_endScope(venv);

                EM_error(a->pos, "lo invalid - integer required");
                assert(0);
                return expTy(NULL, Ty_Nil());
                //return expTy(NULL, Ty_nil);
            }

            //printf("A_forExp F\n");

            // hi has to be int
            struct expty hi_ty = transExp(venv, tenv, hi, inside_loop);

            //printf("ZUZUZUZUZUZUZU\n");
            show_type(hi_ty.ty);
            //printf("\n");
            if (hi_ty.ty->kind != Ty_int)
            {
                S_endScope(venv);

                EM_error(a->pos, "hi invalid - integer required");
                assert(0);
                return expTy(NULL, Ty_Nil());
                //return expTy(NULL, Ty_nil);
            }

            //printf("A_forExp G\n");

            struct expty for_loop_expty = transExp(venv, tenv, body, INSIDE_LOOP);

            S_endScope(venv);

            //printf("A_forExp H\n");

            return for_loop_expty;
        }
        break;

        case A_breakExp:
            //printf("A_breakExp 27\n");
            if (!inside_loop) {
                EM_error(a->pos, "break is not located inside loop flow control structure!");
                assert(0);
            }
            return expTy(a, Ty_Nil());
            //return expTy(NULL, Ty_nil);

        case A_letExp: 
        {
            //printf("A_letExp 28: pos: %d\n", a->pos);

            S_beginScope(venv);
            S_beginScope(tenv);

            A_decList decs = a->u.let.decs;
            while (decs != NULL) {
                //printf("dec: pos: %d\n", a->pos);
                transDec(venv, tenv, decs->head);
                decs = decs->tail;
            }

            struct expty exp;
            A_exp body = a->u.let.body;
            if (body != NULL) {
                //printf("body:\n");
                exp = transExp(venv, tenv, body, inside_loop);
            }

            S_endScope(tenv);
            S_endScope(venv);

            return exp;
        }
        break;

        case A_arrayExp:
        {
            //printf("A_arrayExp 29: pos: %d\n", a->pos);

            // S_symbol typ; A_exp size, init;
            // struct {S_symbol typ; A_exp size, init;} array;
            S_symbol typ = a->u.array.typ;
            A_exp size = a->u.array.size;
            A_exp init = a->u.array.init;

            //printf("typ: \"%s\"\n", S_name(typ));
            //printf("size: %d\n", size->u.intt);
            //printf("init: kind: %d\n", init->kind);

            //printf("AAA\n");

            //printf("\nTAB_DUMP tenv\n=============================\n");
            TAB_dump(tenv, show);
            //printf("=============================\n");

            void* binding_value = S_look(tenv, typ);
            assert(binding_value);
            //printf("binding_value: %d\n", binding_value);

            Ty_ty array_type = (Ty_ty) binding_value;
            show_type(array_type);

            Ty_ty array_element_type = array_type->u.array;
            show_type(array_element_type);

            //printf("BBB\n");

            struct expty init_exp_type = transExp(venv, tenv, init, inside_loop);
            show_type(init_exp_type.ty);

            //printf("CCC\n");

            if (init_exp_type.ty != array_element_type) {
                EM_error(a->pos, "Type used in array initialization does not match array type!");
                assert(0);
                return expTy(NULL, Ty_Nil());
                //return expTy(NULL, Ty_nil);
            } else {
                //printf("Array declaration valid\n");
            }

            return expTy(a, array_type);
        }
        break;

        default:
            //printf("Unknown expression! kind: %d pos: %d\n", a->kind, a->pos);
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
    printf("transVar() %d\n", v->kind);

    switch(v->kind) 
    {
        case A_simpleVar:
        {
            printf("transVar() - simpleVar 12 - VarName: \"%s\"\n", S_name(v->u.simple));

            //printf("\ntransVar() - TAB_DUMP venv\n=============================\n");
            TAB_dump(venv, show);
            //printf("=============================\n");

            Ty_ty ty = TAB_look(venv, v->u.simple);

            //printf("transVar() - VVVVVVVAR is of type: ");
            show(v->u.simple, ty);
            //printf("\n");

            if (ty == NULL) {
                EM_error(v->pos, "Variable \"%s\" is not declared. The type is unknown! Line: %d\n", S_name(v->u.simple), v->pos);
                assert(0);
                return expTy(NULL, Ty_Nil());
                //return expTy(NULL, Ty_nil);
            }

            if (ty->kind == E_varEntry) {
                E_enventry enventry = (E_enventry) ty;
                Ty_ty var_ty = enventry->u.var.ty;
                //printf("transVar() - sssbbb: %d\n", var_ty);
                return expTy(v, var_ty);
            } else if (ty->kind == E_funEntry) {
                assert(0);
            } else {
                return expTy(v, ty);
            }

            assert(0);
        }
        break;

        case A_fieldVar:
        {
            printf("transVar() - A_fieldVar\n");

            // struct {A_var var; S_symbol sym;} field;

            // this is the variable AST node of the record variable that is accessed
            A_var record_variable = v->u.field.var;
            printf("transVar() - record_variable: %d\n", record_variable);
            printf("transVar() - Kind: %d\n", record_variable->kind);

            // this is the field's name that is accessed within the record variable
            S_symbol record_variable_used_field_name = v->u.field.sym;
            printf("transVar() - A_fieldVar 13 - symm: \"%s\"\n", S_name(record_variable_used_field_name));

            Ty_ty record_var_type = NULL;

            if (record_variable->kind == 14)
            {
                printf("Subscript\n");
                struct expty transvar_type = transVar(venv, tenv, record_variable);
                printf("Subscript %d\n", transvar_type);

                record_var_type = transvar_type.ty;
                printf("transVar() - show_type() record_var_type ");
                show_type(record_var_type);
                printf("\n");
            }
            else 
            {
                // this is the name of the variable AST node of the record variable that is accessed
                S_symbol record_variable_name = record_variable->u.simple;

                printf("Symbol: %d\n", record_variable_name);
                printf("transVar() - record_variable_name: %s\n", S_name(record_variable_name));

                // convert record variable name to it's type via the variable environment
                //Ty_ty record_var_type = S_look(venv, record_variable_name);
                record_var_type = S_look(venv, record_variable_name);
                printf("transVar() - show_type() record_var_type ");
                show_type(record_var_type);
                printf("\n");
            }

            if (record_var_type == NULL) {
                // EM_error(v->pos, "Variable \"%s\". Variable is not defined! Line: %d\n", 
                //     S_name(record_variable_name), v->pos);
                assert(0);
            }

            // // DEBUG
            // printf("\ntransVar() - TAB_DUMP venv\n=============================\n");
            // TAB_dump(venv, show);
            // printf("=============================\n");

            // // DEBUG
            // printf("\ntransVar() - TAB_DUMP tenv\n=============================\n");
            // TAB_dump(tenv, show);
            // printf("=============================\n");

            //A_fieldList field_list = var_type->u.record;

            printf("aaaaaaaaaaaaaaaaa \n");
            show_type(record_var_type);
            printf("\n");
            printf("bbbbbbbbbbbbbbbbbb \n");

            // printf("xxxxxxxxxxxxxxxxxx \n");

            // // DEBUG print all the records fields, their names and types
            // //A_fieldList field_list = record_var_type->u.record;
            // E_enventry enventry = (E_enventry) record_var_type;
            // Ty_ty lol = enventry->u.var.ty;
            // A_fieldList field_list = lol->u.record;
            // while (field_list != NULL) 
            // {
            //     A_namety record_field = field_list->head;
            //     printf("transVar() - head: %d\n", record_field);
            //     printf(" name: %s type: %s\n", S_name(record_field->name), S_name(record_field->ty));

            //     // advance iterator
            //     field_list = field_list->tail;
            // }

            // printf("yyyyyyyyyyyyyyy \n");

            //assert(0);

            printf("xxxxxxxxxxxxxxxxxx \n");

            A_fieldList field_list = NULL;

            if (record_var_type->kind == E_varEntry) {

            
            // if the type is taken from a formal parameter, the type is wrapped in an E_enventry
            // if the type is taken from a local or global variable, the type is a normal record type
            E_enventry enventry = (E_enventry) record_var_type;
                Ty_ty unwrapped_type = enventry->u.var.ty;
                field_list = unwrapped_type->u.record;
            } else {
                field_list = record_var_type->u.record;
            }

            printf("field_list: %d \n", field_list);

            while (field_list != NULL) 
            {
                A_namety record_field = field_list->head;
                printf("transVar() - head: %d\n", record_field);
                printf("name: %s type: %s\n", S_name(record_field->name), S_name(record_field->ty));

                //assert(0);

                printf("transVar() - name: %s type: %s\n", S_name(record_field->name), S_name(record_field->ty));

                printf("transVar() - COMPARING record_variable_used_field_name: %s record_field->name: %s\n", 
                    S_name(record_variable_used_field_name), S_name(record_field->name));

                // iterate until the field in the record is found, return it's type
                if (record_variable_used_field_name == record_field->name) {
                    //if (strcmp(S_name(record_variable_used_field_name), S_name(record_field->name)) == 0) {
                    printf("transVar() - returning %s\n", S_name(record_field->ty));

                    Ty_ty type = S_look(tenv, record_field->ty);

                    show_type(type);
                    printf("\n");

                    //return expTy(v, TAB_look(tenv, record_field->ty));
                    return expTy(v, type);
                }

                // advance iterator
                field_list = field_list->tail;
            }

            printf("yyyyyyyyyyyyyyy \n");

            //EM_error(v->pos, "Field \"%s\" is not present in record type! Line: %d\n", S_name(record_variable_used_field_name), v->pos);

            assert(0);
        }
        break;

        case A_subscriptVar:
        {
            // struct {A_var var; A_exp exp;} subscript;

            printf("A_subscriptVar 14\n");
            //printf("A_subscriptVar 14 - VarName: \"%s\"\n", S_name(v->u.subscript.var->u.simple));

            // this is the name of the array type:
            //printf("Array Variable Name: %s\n", S_name(v->u.subscript.var->u.simple));

            // DEBUG
            printf("\ntransVar() - TAB_DUMP tenv\n=============================\n");
            TAB_dump(tenv, show);
            printf("=============================\n");

            // DEBUG
            printf("\ntransVar() - TAB_DUMP venv\n=============================\n");
            TAB_dump(venv, show);
            printf("=============================\n");

            A_var subscript_var = v->u.subscript.var;
            struct expty subscript_var_var = transVar(venv, tenv, subscript_var);
            show_type(subscript_var_var.ty);
            printf("\n");
            printf("aaaa %d\n", subscript_var_var);

            // printf("a\n");
            // Ty_ty array_type = TAB_look(venv, v->u.subscript.var->u.simple);
            // //Ty_ty array_type = TAB_look(venv, subscript_var_var.ty);
            // printf("b %d\n", array_type);

            Ty_ty array_type = subscript_var_var.ty;

            show_type(array_type);

            // subscripts are only allowd on Ty_array
            if (array_type->kind != Ty_array)
            {
                EM_error(v->pos, "Subscript used on variable \"%s\". Variable is not an array! Line: %d\n", S_name(v->u.subscript.var->u.simple), v->pos);
                assert(0);
            }
            
            Ty_ty array_element_type = array_type->u.array;
            show_type(array_element_type);
            printf("\n");

            return expTy(v, array_element_type);
        }
        break;

        default:
            EM_error(v->pos, "Unknown expression!");
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
            printf("transDec: A_varDec\n");

            // // check if the variable is already contained in the current venv
            // void* binding_value = S_look(venv, d->u.var.var);
            // if (binding_value != NULL)
            // {
            //     EM_error(d->pos, "Reuse of variable name \"%s\". This implementation does not allow variable name reuse/shadowing! Line: %d\n", S_name(d->u.var.var), d->pos);
            //     assert(0);
            // }

            // see VARIABLE DECLARATIONS, page 119
            struct expty init_type = expTy(NULL, Ty_Nil());
            //struct expty init_type = expTy(NULL, Ty_nil);

            // determine the type of the initialization value
            if (d->u.var.init != NULL) {

                printf("d->u.var.init - kind: %d\n", d->u.var.init->kind);

                init_type = transExp(venv, tenv, d->u.var.init, OUTSIDE_LOOP);

                printf("init_type retrieved: %d - ", init_type);
                show_type(init_type.ty);
                printf("\n");
            }
            else 
            {
                printf("No initializer\n");
            }

            // the type specifier on a variable is optional so take care
            // here to not dereference a null pointer
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
                printf("Current tenv :\n");

                // DEBUG
                printf("\nTAB_DUMP tenv\n=============================\n");
                TAB_dump(tenv, show);
                printf("=============================\n");

                Ty_ty var_type = S_look(tenv, type_symbol);
                
                printf("type_symbol: \"%s\"\n", S_name(type_symbol));
                show_type(var_type);
                printf("\n");

                bool init_type_is_nil = init_type.ty->kind == Ty_nil;
                bool var_type_is_nil = var_type->kind == Ty_nil;

                printf("init_type_is_nil: %d, var_type_is_nil: %d\n", init_type_is_nil, var_type_is_nil);

                if ((init_type.ty->kind != Ty_nil) && (var_type->kind != Ty_nil)) {

                    printf("AAAAAA\n");

                    printf("var_type: ");
                    show_type(var_type);
                    printf("\n");

                    printf("init_type.ty: ");
                    show_type(init_type.ty);
                    printf("\n");

                    printf("%d - %d\n", var_type, init_type.ty);

                    if (var_type->kind != init_type.ty->kind) {
                        EM_error(d->pos, "Variable \"%s\". Initializer type and variable type differ! Line: %d\n", S_name(d->u.var.var), d->pos);
                        assert(0);
                    }

                    switch (var_type->kind) {

                        case Ty_nil:
                        case Ty_string:
                        case Ty_name:
                        case Ty_void:
                        case Ty_int:
                            break;

                        case Ty_array:
                            if (var_type->u.array != init_type.ty->u.array)
                            {
                                printf("BBBBBB\n");

                                EM_error(d->pos, "Types used in variable declaration initializer are incompatible!");
                                assert(0);
                                //return expTy(NULL, Ty_Nil());
                                //return expTy(NULL, Ty_nil);
                                return;
                            }
                            break;

                        case Ty_record:
                            if (var_type->u.record != init_type.ty->u.record)
                            {
                                printf("CCCCCCCC\n");

                                EM_error(d->pos, "Types used in variable declaration initializer are incompatible!");
                                assert(0);
                                //return expTy(NULL, Ty_Nil());
                                //return expTy(NULL, Ty_nil);
                                return;
                            }
                            break;

                        default:
                            printf("Unknown var type: %d\n", var_type->kind);
                            assert(0);
                    }

                    printf("ahahaahaahh\n");

                    // enter binding for the declared variable into the variable environment (venv)
                    // The initializer type is used because the explizit type specifier is optional and
                    // the initializer and specifier types have to match at all times
                    S_enter(venv, d->u.var.var, init_type.ty);

                    printf("Current venv :\n");

                    // DEBUG
                    printf("\nTAB_DUMP venv\n=============================\n");
                    TAB_dump(venv, show);
                    printf("=============================\n");
                }
                else if (var_type->kind != Ty_nil)
                {
                    S_enter(venv, d->u.var.var, var_type);

                    // DEBUG
                    printf("\nTAB_DUMP venv\n=============================\n");
                    TAB_dump(venv, show);
                    printf("=============================\n");
                } else {
                    assert(0);
                }

                // DEBUG
                printf("\nTAB_DUMP venv\n=============================\n");
                TAB_dump(venv, show);
                printf("=============================\n");
            }
        }
        break;

        case A_typeDec:
        {
            S_symbol type_name = d->u.type->head->name;

            // // check if the type's name is already contained in the current tenv
            // void* binding_value = S_look(tenv, type_name);
            // if (binding_value != NULL)
            // {
            //     EM_error(d->pos, "Reuse of type name \"%s\". This implementation does not allow type_name name reuse/shadowing! Line: %d\n", S_name(type_name), d->pos);
            //     assert(0);
            // }

            printf("A Adding to tenv: \"%s\"\n", S_name(type_name));
            S_enter(tenv, type_name, transTy(tenv, d->u.type));

            // DEBUG
            printf("\nTAB_DUMP tenv\n=============================\n");
            TAB_dump(tenv, show);
            printf("=============================\n");
        }
        break;

        case A_functionDec:
        {
            printf("A_functionDec\n");

            struct expty function_result_type;
            //function_result_type.ty = Ty_nil;
            function_result_type.ty = Ty_Nil();

            A_fundecList fundecList = d->u.function;
            while (fundecList != NULL) 
            {
                A_fundec fundec = fundecList->head;
                printf("fundec: pos: %d\n", fundec->pos);
                printf("fundec: name: %s\n", S_name(fundec->name));

                // // check if the function is already contained in the current venv
                // void* binding_value = S_look(venv, fundec->name);
                // if (binding_value != NULL)
                // {
                //     EM_error(d->pos, "Reuse of function name \"%s\". This implementation does not allow function name reuse/shadowing! Line: %d\n", S_name(fundec->name), d->pos);
                //     assert(0);
                // }

                Ty_tyList formalTys = makeFormalTyList(tenv, fundec->params);

                // functions do not have to return a value. 
                // They can be defined without return type!
                if (fundec->result == NULL)
                {
                    printf("123445\n");
                    S_enter(venv, fundec->name, E_FunEntry(formalTys, NULL));
                }
                else
                {
                    Ty_ty resultTy = S_look(tenv, fundec->result);
                    S_enter(venv, fundec->name, E_FunEntry(formalTys, resultTy));
                }

                //
                // Perform the semantic analysis on the body.
                //
                // Because the body may make use of the formal parameters, 
                // the semantic analysis first creates a new scope for the
                // function body and inserts all formal parameters into it.
                //

                // start parameter scope
                S_beginScope(venv);

                // insert formal parameters into the scope
                {
                    printf("Processing formal function parameters ...\n");

                    A_fieldList l;
                    Ty_tyList t;
                    for (l = fundec->params, t = formalTys; l; l = l->tail, t = t->tail)
                    {
                        printf("param: %s\n", S_name(l->head->name));
                        S_enter(venv, l->head->name, E_VarEntry(t->head));
                    }

                    printf("Processing formal function parameters done.\n");
                }

                // DEBUG
                printf("\n123 TAB_DUMP venv\n=============================\n");
                TAB_dump(venv, show);
                printf("=============================\n");

                // process the body
                printf(">>>+++>>>> Processing function body...\n");
                function_result_type = transExp(venv, tenv, fundec->body, OUTSIDE_LOOP);
                printf("<<<+++<<< Processing function body done.\n");

                // remove parameter scope
                S_endScope(venv);

                printf("testing fundec: %d\n", fundec);
                printf("testing fundec: %d\n", fundec->result);
                printf("testing function_result_type: %d\n", function_result_type);
                printf("testing function_result_type.ty: %d\n", function_result_type.ty);

                if ((fundec->result == NULL) && (function_result_type.ty != Ty_Nil()))
                //if ((fundec->result == NULL) && (function_result_type.ty->kind != Ty_nil))
                {
                    printf("wololo\n");
                    EM_error(d->pos, "Function \"%s\" is declared without return type but returns %s.\n", S_name(fundec->name), label_type(function_result_type.ty));
                    assert(0);
                }

                printf("next function UIOAIUSDOIUASDOIUASDOUIASDUIO\n");

                // next function declaration
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
 * This function takes types and looks them up in the type environment
*/
Ty_ty transTy(S_table tenv, A_ty a)
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
                
                // for all record fields in the record type
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
    printf("\n");
}

void show_type(Ty_ty type)
{
    //printf("show_type\n");

    if (type == NULL)
    {
        printf(" No type!");
        return;
    }
    
    switch (type->kind)
    {
        case E_varEntry:
        {
            printf(" E_varEntry");

            E_enventry enventry = (E_enventry) type;

            show_type(enventry->u.var.ty);
        }
        break;

        case E_funEntry:
        {
            printf(" E_funEntry");

            E_enventry enventry = (E_enventry) type;

            printf(" Result-");
            show_type(enventry->u.fun.result);

            int param_idx = 1;
            Ty_tyList formals = enventry->u.fun.formals;
            while (formals != NULL) {
                
                printf(" Param-%d: ", param_idx);
                param_idx = param_idx + 1;
                show_type(formals->head);
                formals = formals->tail;
            }
        }
        break;

        // 0
        case Ty_record:
        {
            printf(" Ty_record ");

            // print all the records fields, their names and types
            A_fieldList field_list = type->u.record;
            while (field_list != NULL) 
            {
                A_namety record_field = field_list->head;
                printf(" name: %s type: %s", S_name(record_field->name), S_name(record_field->ty));

                // advance iterator
                field_list = field_list->tail;
            }
        }
        break;

        case Ty_nil:
            printf("Ty_nil");
            break;
            
        case Ty_int:
            printf("Ty_int");
            break;
            
        case Ty_string:
            printf("Ty_string");
            break;
            
        case Ty_array:
            printf("Ty_array");
            break;
            
        case Ty_name:
            printf("Ty_name");
            break;
            
        case Ty_void:
            printf("Ty_void");
            break;
            
        default:
            printf("Unknown type!");
            break;
    }
}

char* label_type(Ty_ty type) {
    if (type == NULL)
    {
        return "No type!";
    }
    
    switch (type->kind)
    {
        case E_varEntry:
        {
            E_enventry enventry = (E_enventry) type;
            return label_type(enventry->u.var.ty);
        }
        break;

        case E_funEntry:
        {
            E_enventry enventry = (E_enventry) type;
            return label_type(enventry->u.fun.result);

            // int param_idx = 1;
            // Ty_tyList formals = enventry->u.fun.formals;
            // while (formals != NULL) {
                
            //     printf(" Param-%d: ", param_idx);
            //     param_idx = param_idx + 1;
            //     show_type(formals->head);
            //     formals = formals->tail;
            // }
        }
        break;

        // 0
        case Ty_record:
        {
            return "Ty_record";
            // // print all the records fields, their names and types
            // A_fieldList field_list = type->u.record;
            // while (field_list != NULL) 
            // {
            //     A_namety record_field = field_list->head;
            //     printf(" name: %s type: %s", S_name(record_field->name), S_name(record_field->ty));

            //     // advance iterator
            //     field_list = field_list->tail;
            // }
        }
        break;

        case Ty_nil:
            return "Ty_nil";
            break;
            
        case Ty_int:
            return "Ty_int";
            break;
            
        case Ty_string:
            return "Ty_string";
            break;
            
        case Ty_array:
            return "Ty_array";
            break;
            
        case Ty_name:
            return "Ty_name";
            break;
            
        case Ty_void:
            return "Ty_void";
            break;
            
        default:
            return "Unknown type!";
            break;
    }
}