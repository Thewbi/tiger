/*
 * table.c - Functions to manipulate generic tables.
 * Copyright (c) 1997 Andrew W. Appel.
 */

#include <stdio.h>
#include "util.h"
#include "table.h"

#define TABSIZE 127

typedef struct binder_ *binder;
struct binder_ {void *key; void *value; binder next; void *prevtop;};
struct TAB_table_ {
  binder table[TABSIZE];
  void *top;
};


static binder Binder(void *key, void *value, binder next, void *prevtop)
{binder b = checked_malloc(sizeof(*b));
 b->key = key; b->value=value; b->next=next; b->prevtop = prevtop; 
 return b;
}

TAB_table TAB_empty(void)
{ 
 TAB_table t = checked_malloc(sizeof(*t));
 int i;
 t->top = NULL;
 for (i = 0; i < TABSIZE; i++)
   t->table[i] = NULL;
 return t;
}

/* The cast from pointer to integer in the expression
 *   ((unsigned)key) % TABSIZE
 * may lead to a warning message.  However, the code is safe,
 * and will still operate correctly.  This line is just hashing
 * a pointer value into an integer value, and no matter how the
 * conversion is done, as long as it is done consistently, a
 * reasonable and repeatable index into the table will result.
 */

void TAB_enter(TAB_table t, void *key, void *value)
{
    // printf("TAB_enter() key \"%s\"\n", S_name(key));
    int index;
 assert(t && key);
 index = ((unsigned)key) % TABSIZE;
 t->table[index] = Binder(key, value, t->table[index], t->top);
 t->top = key;
}

/**
 * @param key has to be of type Symbol (e.g. S_Symbol("int"))
 * @returns Ty_ty
*/
void *TAB_look(TAB_table t, void *key)
{
    // printf("TAB_look() key: %d\n", key);
    // printf("TAB_look() for \"%s\"\n", S_name(key));

    int index;
    binder b;

    assert(t && key);

    index = ((unsigned) key) % TABSIZE;
    for (b = t->table[index]; b; b=b->next)
    {
        // printf("b->key: %s\n", S_name(b->key));

        // if (b->key == "<mark>")
        // {
        //     printf("Found Marker!!!!!!!!!!!!");
        //     //break;
        // }

        if (b->key == key) 
        {
            // printf("Type \"%s\" found! Binding is: %d\n", S_name(key), b->value);
            // show(key, b->value);
            // printf(".........\n");
            return b->value;
        }
    }

    // printf("Type \"%s\" not found!\n", S_name(key));
    return NULL;
}

void *TAB_pop(TAB_table t) {
  void *k; binder b; int index;
  assert (t);
  k = t->top;
  assert (k);
  index = ((unsigned)k) % TABSIZE;
  b = t->table[index];
  assert(b);
  t->table[index] = b->next;
  t->top=b->prevtop;
  return b->key;
}

/**
 * The hashtable is a hashtable of binders.
 * 
 * The binders themselves have pointers to one other binder.
 * Using this chain of binders, a "stack" of binders is emulated.
 * The table stores a pointer to the current top of the stack binder.
 * 
 * This "stack" of binders is used to implement the destructive nature for
 * type scopes! When a type scope starts, a <mark> is inserted into the binder chain.
 * Within the scope, new types are placed on top of the mark. When the scope ends,
 * all types up to the next mark (including the mark itself) are erased. This is how
 * type scopes are implemented. The erasing part is the reason why this strategy is
 * called destructive.
 */
void TAB_dump(TAB_table t, int recursion_depth, void (*show)(void *key, void *value, int recursion_depth)) {

    if (recursion_depth == 0)
    {
        return;
    }

    // store the key of the topmost binder into k
    void *k = t->top;

    // retrieve the binder which is hashed for the key top_binder_key
    int index = ((unsigned)k) % TABSIZE;
    binder b = t->table[index];

    // abort condition for the recursion
    if (b == NULL) {
        return;
    }

    // decend into next element
    t->table[index] = b->next;
    t->top = b->prevtop;

    // show the current type
    show(b->key, b->value, recursion_depth);

    // recurse
    TAB_dump(t, recursion_depth, show);

    assert(t->top == b->prevtop && t->table[index] == b->next);

    // revert all the changes made in this level of recursion
    t->top = k;
    t->table[index] = b;
}

/**
 * Decends the (emulated) stack of types in the current scope (= until the first <mark>) only
 * and performs resolution of mutually recursive types. This means that the placeholder
 * Name(s, NULL) is replaced by the real type! If the real type is not defined, a semantic error
 * has been found!
 */
void TAB_resolve_mutually_recursive_types(int pos, TAB_table tenv, int recursion_depth, bool throw_error_on_undefined_type, void (*show)(void *key, void *value, int recursion_depth)) 
{
    //printf("TAB_resolve_mutually_recursive_types\n");

    // store the key of the topmost binder into k
    void *top_binder_key = tenv->top;

    while (top_binder_key != NULL)
    {
        // retrieve the binder which is hashed for the key top_binder_key
        int index = ((unsigned)top_binder_key) % TABSIZE;
        binder top_binder = tenv->table[index];
        if (top_binder == NULL) {
            return;
        }

        //printf("Key: %s\n", S_name(top_binder->key));

        // detect the end of the current scope (first <mark> binder)
        if (strcmp("<mark>", S_name(top_binder->key)) == 0) 
        {
            //printf("Aborting TAB_fix_mutually_recursive_types at first <mark>\n");
            return;
        }

        //printf("show...\n");

        // show the current type
        //show(top_binder->key, top_binder->value, recursion_depth);

        //printf("show done.\n");

        // find records, because the resolution is implemented for record fields!
        Ty_ty binder_value = top_binder->value;
        if ((binder_value != NULL) && (binder_value->kind == Ty_record)) {

            //printf("Record found!\n");

            // print all the records fields, their names and types
            Ty_fieldList field_list = binder_value->u.record;
            while (field_list != NULL) 
            {
                //printf("A\n");
                Ty_field record_field = field_list->head;

                //printf("B\n");
                
                // for (int i = 0; i < 1; i++)
                // {
                //     printf("\t");
                // }
                // printf("[name: %s", S_name(record_field->name));
                // printf(" type: ");

                // // DEBUG
                // show_type_indent(record_field->ty, 0);
                // printf("\n");

                if (record_field->ty->kind == Ty_name) {
                    //printf("I HAVE TO REPLACE THIS S_NAME BY THIS TYPE: %s", S_name(record_field->ty->u.name.sym));

                    void* binding_value = S_look(tenv, record_field->ty->u.name.sym);
                    if (binding_value == NULL)
                    {
                        // if a type cannot be resolved yet, then just advance.
                        // The type is resolved later, if the file is semantically correct

                        if (throw_error_on_undefined_type) {
                            //printf("Could not resolve\n");

                            EM_error(pos, "Could not resolve type \"%s\"!", S_name(record_field->ty->u.name.sym));
                            assert(0);
                        }

                        // EM_error(0, "undeclared type \"%s\". tenv does not contain type: \"%s\"\n", S_name(record_field->ty->u.name.sym), S_name(record_field->ty->u.name.sym));
                        // assert(0);

                        // advance iterator
                        field_list = field_list->tail;

                        continue;
                    }

                    // replace placeholder by resolved actual type
                    Ty_ty resolved_type = (Ty_ty) binding_value;
                    record_field->ty = resolved_type;
                }

                //printf("]\n");

                if (field_list == field_list->tail)
                {
                    return;
                }

                // advance iterator
                field_list = field_list->tail;
            }
        }

        // ???
        //t->table[index] = top_binder->next;

        if (top_binder_key == top_binder->prevtop)
        {
            return;
        }

        // descend the emulated stack of type definitions
        top_binder_key = top_binder->prevtop;
    }
}
