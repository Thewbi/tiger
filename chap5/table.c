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

void TAB_dump(TAB_table t, void (*show)(void *key, void *value)) {
  void *k = t->top;
  int index = ((unsigned)k) % TABSIZE;
  binder b = t->table[index];
  if (b==NULL) return;
  t->table[index]=b->next;
  t->top=b->prevtop;
  show(b->key,b->value);
  TAB_dump(t,show);
  assert(t->top == b->prevtop && t->table[index]==b->next);
  t->top=k;
  t->table[index]=b;
}
