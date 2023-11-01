# Chapter 5 on Semantic Analysis

This chapter is a little bit confusing for a student that has never had contact with semantic analysis before. 
It is confusing because if one has no experience with a subject, the lack of orientation leads to confusion.
The confusion has to be resolved by determinig the start and the end goal, namely by gaining the orientation 
that is missing.

When the current situation (start)
is understood and the end goal is understood, then the student can begin to inch their way forward to close
the gap between the current state and the goal. The feeling of beeing lost is gone and some kind of routine
can be achieved which gives the student a feeling of beeing in charge of the situation.

The strategy is to first understand the goals to achieve in this chapter.
Then, once the goals are clear, the next step is to understand the datastructures used to achieve the goal.
The last step is to connect the parser via it's exp output to the semantic analysis implementation and to
test the output of the semantic analysis phase. Therefore a executable application is created (semanttest.exe)
which puts all pieces together.

## Original Files

The files contained in this folder originally are types.h and types.h.
They contain the types (int, string, name, record, array, nil, void) out of which all types in the
tiger language can be constructed.

The types will go into the tenv which is a datastructure filled in by the semantic analysis phase
as it traverses the AST recursively. The contents are the types and the location of where they have
been defined.

The functions that perform the semantic analysis AST traversal are contained in the files semant.h and
semant.c. It is the students task in chapter 5 to implement the semant module (semant.h and semant.c)

Pretty much all files to achieve this goal can be copied over from previous chapters.
The semant.h and semant.c have to be created manually as they are not part of the book's complementary
material.

## Goals of Semantic Analysis

The actual goal of chapter 5 is to implement the files semant.h, semant.c which form the semant module (see 
page 115 of the book.) Those files are not provided in the book complementary material, you have to create them
yourself. Snippets in the book give a framework on which you can base your implementation. The code mentioned
below is taken from the book.

The semant module will contain the functions (see page 115)

```
struct expty transVar(S_table venv, S_table tenv, A_var v);
struct expty transExp(S_table venv, S_table tenv, A_exp a);
void transDec(S_table venv, S_table tenv, A_dec d);
struct Ty_ty transTy ( S_table tenv, A_ty a);
```

The trans (translate) prefix is due to the fact that these functions are extended later from semantic analysis to also perform
intermediate code generation. Therefore a general name was choosen and trans stands for translate.

trans functions will walk the AST recursively and they will first perform semantic analysis.
Therefore they get two environments as parameters. One enviroment is the variable and function environment (venv),
the other one is the type environment (tenv). Two distinct environments are needed because of applications such as

```
let 
    type a = int
    var a : a := 5
    var b : a := a
in 
    b + a
end
```

Here the identifier a is used as for a type and also for a variable. In contexts where a type is expected, a will 
be looked up in the tenv. In contexts where variable or functions are expected, a will be retrieved from the venv.
But it is valid to have a duplicate use of the symbol a for type and variables/functions.

The type S_table is given in the book's complementary code. S_table is the datastructure that is used to 
maintain the two environments tenv and venv. The S_table data structure is discussed below.

Firstly chap4/table.h and chap4/table.c defined the type *struct TAB_table_* then symbol.h defines the type S_table
using struct TAB_table_.

```
typedef struct TAB_table_ *S_table;
```

### Type checking

The goal is to maintain symbol tables that map identifiers / symbols to their type and location of where they are declared.

Symbols (Identifiers) in the tiger language are variables, types and functions.
The concrete values stored inside the variables symbols are of no interest during semantic analysis, their type is of interest.

When a variable gets a value assigned, semantic analysis has to check if the assigned value is of the same or a compatible type as
the symbol that is assigned a value to. Compatible means that there must be automatic type cast rules backed into 
the language definition or the user has explicitly applied a cast. The same is true for initializing a struct. All values
used in the initializer have to be compatible with the respective struct fields.

When a function is called, the values if any passed into the function have to be compatible with the function's formal
parameter list defined in the functions definition.

### Environments / Scopes / Lifetime

Each mapping / entry into the symbol tables has a scope. When the scope ends, the mapping has to be invalidated (which
can be implemented in several ways).

## Data Structures

### Destructive / Imperative

On page 106, the data structure "Hash table with external chaining" is introduced.
The example code is very detailed but in the end, this data type is not used in the remainder of the
book! Instead the files (symbol.h/c and table.h/c contain the real datatypes used to implement Symbol Tables)

First of all, the data structure uses a void pointer for bindings. The reason for not
specifiying a concrete type for a binding is that the datastructure has to work with different 
types of bindings.

The datastructure has 109 entries for some unknown reason.
There is a hash function that converts a string (used with identifiers / symbols) to a hash code.
Wherever the hash function is used, the resulting code is module divided by the tables size of 109.
That way, the hash can always be used as an index into the hash table.

Each entry in the hash table is an element of a linked list. The bucket struct is used as the implementation
of the linked list. This means that the table can contain several entries per hash. Whenever two values are 
mapped to the same index, then the list inside the table at that index is just extended by a new bucket element.

Here is how to implement Create, Retrieve, Update and Delete (CRUD) operations for this datatype.

Create - A binding is created (Mapping of identifier to type). If a binding should be inserted into the table, 
you need to use the symbol / identifier of the binding as key and the binding itself as binding parameter to
the insert() call. insert() will then compute the index into the hashtable where to store the new entry.
It will instantiate a new bucket and replace the bucket stored in the current table element by the newly
created bucket. The newly created bucket will link to the prior bucket to extend the linked list by prepending itself.
There is a problem with insert() in that it does not check if the table already contains a bucket for the key!
The user has to call lookup() to check if such a binding exists already.

Retrieve - use the lookup call using a key as parameter which returns a binding or NULL when no matching bucket
can be found. The key is hashed to find the tables index and the bucket stored at that index. Since several 
different strings can be hashed into the same bucket, the next step is to traverse the linked list and to find
the bucket that stores the identifier. The first found bucket matching the key is then returned.

Update - there is no implementation of update

Delete - pop() is used to remove the first bucket in the table where the key parameter hashes to.
In my opinion, this implementation is incorrect since it does not traverse the linked list to remove
the correct bucket but it just erases the first bucket! The first bucket is not necessarily the bucket
that stores the key! The only way in which the pop() function can work correctly is when a stack like
data structure is used to keep track of the order in which new buckets have been added to the hash map.
In this context, the functions name (pop()) makes a lot more sense. If variables are removed (poped) from
the hashtable in the exact reverse order in which they have been added, the the pop() function will
not make any mistakes and will exactly remove the elements from the list that should be removes since
they are at the beginning of the linked list. The stack makes it possible to get rid of symbols
in the reverse order in which they have been added.

Using a stack and the "Hash table with external chaining" it is possible to create environments (Scopes/Lifetimes)
in a destructive way or imperative as Mr. Apple calls it in his book. Destructive and imperative means that there 
is a single environment that is modified to go to the new environment for situations in which environments nest. 
Then by popping the added bindings out of the environment in reverse order (requires a stack), that they have been
added in, the old environment is reinstalled again.

### Non-Destructive / Functional

The functional way according to Mr. Appel is to keep several environments alive at the same time. The task now
is to find a datastructure that allows the creation and destruction of environments in an efficient manner.

To have efficient functional management of environments, the data structure of binary search trees are proposed.
Especially the persistent red-black tree is mentioned.

There is no full implementation of Non-Destructive Environments using persistent red-black trees in the book (I think at least).
This has to be added by the reader.

### The Symbol and Table type actually used (S_Table)!

The goal here seems to have a Symbol data structure that maps the same identifier to the one Symbol instance, no matter how
often a identifier is used in the application.

The files symbol.h and symbol.c are actually provided in the folder of chapter 4 but not in the folde of chapter 5 for some
reason. I have copied the files over without any modification.

symbol.h makes use of the TAB_table_ data type which is stored in chap4/table.h and chap4/table.c.
I copied the files over into the chap5 folder.

Table is used as the actual implementation of the symbol table in the remainder of the book.
Table is a desctructive / imperative table that uses a stack to roll and unroll environments as discussed before.

# Structure of the Semantic Analysis Module semant.h/semant.c

After a bit of trial-and-error with the semant.h/semant.c, the following can be said about the overall approach.

Firstly, there have to be two environments prepare in the semanttest.c like so:

```
S_table venv = S_empty();
S_table tenv = S_empty();
```

to prevent errors from happening. The important part is to call S_empty() so that the end pointers of linked
lists are set to NULL correctly. venv is the environment for variables and functions. tenv is the environments for types.

To output the tables, the function TAB_dump() from table.h/c is used. The function pointer points to a
user defined function that performs the output. One skeleton could be:

```
/**
 * Called by TAB_dump() in table.c/h
 * Outputs a binding stored inside a table environment
 * 
 * @param: key is a symbol that can be printed with S_name(key) which converts the symbol into a string
 * @param: value E_VarEntry
 */ 
void show(void *key, void *value)
{
    printf("Key: '%s' ", S_name(key));

    E_enventry env_entry = (E_enventry)value;
    if (E_varEntry == env_entry->kind)
    {
        Ty_ty type = env_entry->u.var.ty;
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
    else
    {
        printf("Unknown kind: %d:\n", env_entry->kind);
    }
}
```

I have placed show() into semant.h/.c since it is used there exclusively. You can use TAB_dump()
to check your intermediate results ant to check what bindings from symbol name to type are stored in
your environments.

Semant is recursive in that transExp() calls itself and every execution of the semantic analysis phase starts with a call
to transExp(). transExp() performs a depth-first traversal over the AST. Therefore it will encounter variable declarations
and variable usages. (And also type declarations and function declarations).

transExp() will fill the environments with entries when it encounters variable declarations. It will then access the
environments for type checking when it encounters variable usages.

The entry of a binding into an environment is performed in a let expression for example:

```
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
```

The let environment has a declaration section where variables are declared. For each dec in the A_decList, transDec()
is called. transDec() is part of the semantic analysis module and it enters a binding into the venv (variable enviroment)
using the S_enter() call. I also print the environment using TAB_dump() whenever a binding is added to the environment.

```
/**
 * transDec - (Trans)late (Dec)larations
 * 
 * Processes
 * - Variable Declarations (e.g. var a:int := 2)
 */
void transDec(S_table venv, S_table tenv, A_dec d)
{
    printf("transDec: pos: %d\n", d->pos);

    // see VARIABLE DECLARATIONS, page 119

    // determine the type of the initialization value
    struct expty e = transExp(venv, tenv, d->u.var.init);

    S_enter(venv, d->u.var.var, E_VarEntry(e.ty));

    TAB_dump(venv, show);

    printf("transDec done.\n");
}
```

When a + operator is encountered for example, transExp() recursively decends into both branches for the
left and the right operand.

```
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
```

The recursive calls to transExp()

```
struct expty left = transExp(venv, tenv, a->u.op.left);
struct expty right = transExp(venv, tenv, a->u.op.right);
```

have to return *struct expty* values. These struct expty values are the types that
have been inserted into the venv by transDec() earlier! So here is how entries from the venv are
retrieved when transExp() encounters a variable usage:

```
case A_varExp:
    printf("A_varExp 15\n");
    return transVar(venv, tenv, a->u.var);
```

transVar() will retrieve a binding for the used variable by calling TAB_look():

```
struct expty transVar(S_table venv, S_table tenv, A_var v)
{
    printf("transVar\n");

    switch(v->kind) 
    {
        case A_simpleVar:
            printf("simpleVar 12 - VarName: \"%s\"\n", S_name(v->u.simple));
            E_enventry env_entry = TAB_look(venv, v->u.simple);
            return expTy(NULL, env_entry->u.var.ty);

    ...
```

this returned struct expty is the type of the variable that is used in the operator.
This type is then used to check if both operands to the plus operator are of type int!

```
if (left.ty->kind != Ty_int)
{
    EM_error(a->u.op.left->pos, "integer required");
}

if (right.ty->kind != Ty_int)
{
    EM_error(a->u.op.right->pos,"integer required");
}
```

Now the operator itself returns a type of int:

```
return expTy(NULL, Ty_Int());
```

This type can then be further used in the recursion.

# Approach Developing and Testing the Software for Chapter 4

## Variable Declarations, Variable Initialization

Tiger is strictly typed. Variables always have to have a type.
The type of a variable is determined once during it's declaration.

The variable declaration always has to contain an assignment.
The variable declaration optionally can contain a type specification.

```
let
    var a := 0
	var i:int := 5
in
	0
end
```

The type specification will determine the type of the variable and the assigned
value's type has to match it. If there is no type specified, then the type of
the assigned value is used as the type of the declared variable.

```
let
	var i:int := 5
    var a := 0
    var b := "test"
in
	i
end
```

Dumping the variable environment (venv) yields:

```
Key: 'b' Type: Ty_string
Key: 'a' Type: Ty_int
Key: 'i' Type: Ty_int
```

In this example, the specified type does not match the initialization value's type.
The semantic analysis has to return an error.

```
let
	var i:int := "this will err"
in
	0
end
```

### Semantic analysis detects errors here
test31.tig
test31.tig has to fail since the declared type and the type of the value that is used during initialization differs.

### Semantic analysis correct here

## Variable Assignments

Assignments where the left hand side (lhs) type is not known have to fail!

```
i := 1
```

Here, i is of unknown type because there is no variable declaration anywhere.





## Type declarations

### Alias for built-in types

The most basic type declaration uses the built-in types 'int' and 'string' to create
a type that is nothing but an alias to the built-in types 'int' and 'string'.

```
let
	type myint = int
in
	0
end
```

Question: Can a variable of type myint be assigned a int value or assigned a variable of type int?

### Array Types

One step up in complexity is the array type:

```
let
	type arrtype1 = array of int
    type arrtype2 = array of string
in
	0
end
```

### Records

Next are records.

```
let
	type rectype = { name:string, age:int }
in
	0
end
```

typedec_simple.tig
test29.tig
arrays_simple.tig
records_simple.tig


## Arithmetic Operators

```
semanttest.exe ..\testcases\addition.tig
```

addition.tig looks like this

```
let
    var a:int := 2
    var b:int := 3
    var c:int := 0
in
	c := a + b
end
```

The test is successfull if the plus operator test determines that both operands are of type int and declares
the operation sane. The test also has to determine that the assignment of int to c is valid and sane.

## Arrays

In the tiger programming language, using arrays is a three-step process.

1. define a (array) type
1. declare a variable of the (array) type. Optionally initialize all fields during declaration.
1. assign values to fields of the array variable or access values in fields of the array variable

An example is

```
let
	type arrtype1 = array of int

	var arr1: arrtype1 := arrtype1 [10] of 0
in
	arr1[0] := 123;
    arr1[2]
end
```

test29.tig type declaration based on arrays.

It is invalid to index a variable that is not an array (test24.tig)

```
/* error : variable not array */
let 
	var d:=0
in
	d[3]
end
```

### Semantic analysis detects errors here
test31.tig

### Semantic analysis correct here
TODO: arrays_simple.tig

## Records / Structs

It is invalid to perform field access on variables that are not structs / records (test25.tig).

```
/* error : variable not record */
let 
	var d:=0
in
	d.f 
end
```

test28.tig different record types. Tiger defines record equality based on record declaration not 
record structure. Two structurally equal records are not equal because they are two distinct records.

## Let-Scopes

As outlined in chapter 5.4 TYPE-CHECKING DECLARATION on page 118, entering a let environment has to 
lead to S_beginScope() and ultimately to S_endScope() calls for both the venv and the tenv. Using these
calls, scopes are implemented and variable can be shadowed.

## Functions

TODO