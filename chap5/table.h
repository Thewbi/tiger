#ifndef _TABLE_H_
#define _TABLE_H_

#include "symbol.h"
#include "types.h"

/*
 * table.h - generic hash table
 *
 * No algorithm should use these functions directly, because
 *  programming with void* is too error-prone.  Instead,
 *  each module should make "wrapper" functions that take
 *  well-typed arguments and call the TAB_ functions.
 */

typedef struct TAB_table_ *TAB_table;

/* Make a new table mapping "keys" to "values". */
TAB_table TAB_empty(void);

/* Enter the mapping "key"->"value" into table "t", 
 *    shadowing but not destroying any previous binding for "key". 
 *
 * @param t
 * @param key
 * @param value
 */
void TAB_enter(TAB_table t, void *key, void *value);

/**
 *  Look up the most recent binding for "key" in table "t"
 * @param key has to be of type S_Symbol (e.g. S_Symbol("int"))
 * @returns is of type Ty_ty
*/
void *TAB_look(TAB_table t, void *key);

/* Pop the most recent binding and return its key.
 * This may expose another binding for the same key, if there was one. */
void *TAB_pop(TAB_table t);

/* Call "show" on every "key"->"value" pair in the table,
 *  including shadowed bindings, in order from the most 
 *  recent binding of any key to the oldest binding in the table */
void TAB_dump(TAB_table t, int recursion_depth, void (*show)(void *key, void *value, int recursion_depth));

/**
 * Decends the (emulated) stack of types in the current scope (= until the first <mark>) only
 * and performs resolution of mutually recursive types. This means that the placeholder
 * Name(s, NULL) is replaced by the real type! If the real type is not defined, a semantic error
 * has been found!
 */
void TAB_resolve_mutually_recursive_types(TAB_table t, int recursion_depth, void (*show)(void *key, void *value, int recursion_depth));

#endif