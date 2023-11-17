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

## The unit type

See https://www.haber-kucharsky.com/proj/fang/tiger.html

The unit type is used for functions that return no value and are merely called because their side-effects are beneficial.
The unit type is also created by if-then statements that lack an else branch or when a variable is assigned an empty sequence:

test43.tig  ( semanttest.exe ..\testcases\book\test43.tig )

```
/* initialize with unit and causing type mismatch in addition */

let 
	var a := ()
in
	a + 3
end
```

There is no explicit type defined for unit. The semantic analysis in this compiler returns NULL or Ty_Nil() for the unit.

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

After a bit of trial-and-error with the semant.h/semant.c files, the following can be said about the overall approach.

Firstly, there have to be two environments prepared in the semanttest.c like so:

```
S_table venv = S_empty();
S_table tenv = S_empty();
```

to prevent seg-faults from happening. The important part is to call S_empty() so that the end pointers of linked
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
to check your intermediate results and to check what bindings from symbol name to type are stored in
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






# Approach Developing and Testing the Software for Chapter 5

## Basic Types (int, string)

### Variable Declarations, Variable Initialization of Basic Type (int, string)

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

### Type Shadowing and Variable Shadowing

When declaring types, the type declarations form "batches".
A batch is a consecutive list of type decalarations:

```
type a = int
type a = string
```

A batch is interrupted by variable declarations. After the 
variable declaration, the next batch starts:

```
type a = int    // type batch 1
var b := 4      // variable batch 1
type a = string // type batch 2
```

A language feature in tiger says: 
It is allowed to redeclare a type accross two distinct batches! 
It is *not* allowed to redeclare a type within the same batch!

Make sure that redeclaring a variable is allowed if and only if, the
two batches are separated by a declaration or something else.

THIS IS ILLEGAL:

test38.tig ( semanttest.exe ..\testcases\book\test38.tig )

```
/* This is illegal, since there are two types with the same name
    in the same (consecutive) batch of mutually recursive types. 
    See also test47  */
let
    type a = int
    type a = string
in
    0
end
```

THIS IS LEGAL:

test47.tig ( semanttest.exe ..\testcases\book\test47.tig )

```
/* This is legal. The second type "a" simply hides the first one.
   Because of the intervening variable declaration, the two "a" types
   are not in the same batch of mutually recursive types.
   See also test38 */
let
    type a = int
    var b := 4
    type a = string
in
    0
end
```

NOTE: 
I do not really understand what added-value this constraint of batches adds to the language. 
The current semantic analysis does not care about specific declaration batches! 
Variables are shadowed whenever a name is redeclared!

The shadowing has the following effect on the current tenv or venv 
(since it is the same for variable not only for types.)

If this code is compiled: test47.tig,         ( semanttest.exe ..\testcases\book\test47.tig )

```
let
    type a = int
    var b := 4
    type a = string
in
    0
end
```

first, type a is an alias for int. The tenv will look like this:

```
TAB_DUMP tenv
=============================
Key: 'a' Ty_int
<mark>
Key: 'string' Ty_string
Key: 'int' Ty_int
=============================
```

Then a variable b is defined and directly after it, type a is redefined to be an alias of string.
The tenv will look like this:

```
TAB_DUMP tenv
=============================
Key: 'a' Ty_string
Key: 'a' Ty_int
<mark>
Key: 'string' Ty_string
Key: 'int' Ty_int
=============================
```

The important point is that it is perfectly correct to have two entries for a in the current tenv
as long as when the code is generated, the first, top-most entry for a is used because it shadows
all the other definitions below it!

It would be an error to use the lower a (which is an alias to int) because then code is generated
for a type that the programmer has shadowed!

In other words, the retrieval (S_look()) on tables has to work correctly. When asked for a type
or variable, S_look() has to return the most current entry shadowing all other entries.

A very, very bad downside of shadowing in my opinion occurs when code is separated into modules.
There are three types of modularization in modern programming languages as I see it.

1. Modularization into several files pre compilation (combining files using a preprocessor and include statements)
2. Modularization between compilation units (object files) (combining them using a linker)
3. Modularization into libraries (static .lib or dynamic .dll)

The problem arises in all cases I think.

If a tiger programm shadows a function, then it might fundamentally change the way a library works!
Imagine the case where a library managing network traffic uses a function called store_to_buffer().

Imagine a tiger application shadowing store_to_buffer() for it's own purpose not knowing that store_to_buffer()
has an important meainging in another context that it does not even know of, namely the network management routines.
Then imagine after shadowing store_to_buffer(), the application uses the network stack which eventually
calls into the store_to_buffer() routine that previously did it's job in the context of the networking code
but now has been shadowed and repurposed to do something else that makes sense in the context of the application
but breaks the network stack!

The nasty thing is that finding this type of bug in an application that is put together by the compiler or linker
later, after the programmer has tested their code thoroughly is very hard. The programmer says: I have written
one million unit tests, my code is not that bugged, the bug must be in the network stack. The network stack 
programmer says: Our code is used in thousands of system since the year 1837, it is not bugged! The bug must be 
in your code. In the end, the bug is not a bug but a feature. The language allows you to effectively disable
code contained in other modules!

I think that shadowing also has an upside. Imaging you want to extend a method contained in a library.
You can just shadow that function with your extended implemenation and get more debugging output that way
or have the library do new things.






### Variable Assignments of Basic Type (int, string)

Assignments where the left hand side (lhs) type is not known have to fail!

```
i := 1
```

Here, i is of unknown type because there is no variable declaration anywhere.





## Type declarations

### Alias for built-in types

The most basic type declaration uses the built-in types 'int' and 'string' to create
a type that is nothing but an alias to the built-in types 'int' and 'string'.

An example is in typedec_simple.tig:

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

arrays_simple.tig

```
let
    type arrtype1 = array of int
    type arrtype2 = array of string
in
    0
end
```

arrays_simple.tig
arrays.tig
test24.tig
test29.tig

### Records

Next are records.

```
let
    type rectype = { name:string, age:int }
in
    0
end
```

records_simple.tig
records_var_declaration.tig
records_field_assignment.tig
book\test3.tig

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

1. define a (array) type (See type declarations above)
1. declare a variable of the (array) type. Optionally initialize all fields during declaration.
1. assign values to fields of the array variable or access values in fields of the array variable

An example is arrays.tig:

arrays.tig

```
let
    type arrtype1 = array of int
    var arr1: arrtype1 := arrtype1 [10] of 0
in
    arr1[0] := 123;
    arr1[2]
end
```

Another example is arrays_simple.tig

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

## if-then-else

Here is the definition of if-then and if-then-else from page 524 of the book:

```
If-then-else: The if-expression 'if exp1 then exp2 else exp3' evaluates the integer expression exp1. 
If the result is nonzero it yields the result of evaluating exp2; otherwise it yields the result of exp3. 
The expressions exp2 and exp3 must have the same type, which is also the type of the entire if-expression 
(or both expressions must produce no value).
```

```
If-then: The if-expression 'if exp1 then exp2' evaluates the integer expression exp1. 
If the result is nonzero, then exp2 (which must produce no value) is evaluated. 
The entire if-expression produces no value.
```

One important question to ask is why do if-then and if-then-else statements even return values?
The answer is that the tiger programming language does not have a return keyword. The return value
of a function body is the value that the last statement returns. 

If an if-then-else statement is used as the last expression in a function body it has to return a value!
If an if-then statement is used as the last expression in a function, the function is not allowed to
have a return value since the book defines that the if-then statement does not return a type!

Looking at a function declaration:

```
function f(i:int) = 
    if i>0 then 
    (
        f(i/10); 
        print(chr(i-i/10*10+ord("0")))
    )
```

here, the last statement is an if-then. The if-then statement does not return a value so the function f can
not return a value.

Looking at another function:

```
type list = {first: int, rest: list}

function readlist() : list =
    let 
        var any := any{any=0}
        var i := readint(any)
    in 
        if any.any
        then list{first=i,rest=readlist()}
        else nil
    end
```

The last statement is the if-then-else and it has to either return nil of a list record.

### Semantic Analysis for if-then and if-then-else

The then and else branches have to return the same type. 
This is illegal:

```
if (5>4) then 13 else " "
```

An example for a if-then-else statement that produces no value is

```
if l=nil then 
    print("\n")
else 
    (printint(l.first); 
    print(" "); 
    printlist(l.rest))
```

The last three statements form a sequence and are all executed by the else-branch!

It is illegal to return a non-unit as in test15.tig, meaning
that the the if-then statement (the variant without an else) is not allowed to produce a type!

```
/* error : if-then returns non unit */

if 20 then 3
```

A valid if-then is:
It does not produce a type since, print does not return a type.

```
if i>0 then 
(
    f(i/10); 
    print(chr(i-i/10*10+ord("0")))
)
```

It is illegal to compare two nil values since nil should only be used where
at least one concrete type is present.
```
if nil = nil then ...
```

test4.tig,          ( semanttest.exe ..\testcases\book\test4.tig ) // OK
test8.tig,          ( semanttest.exe ..\testcases\book\test8.tig & cat ast_dump.txt ) // OK
test9.tig,          ( semanttest.exe ..\testcases\book\test9.tig & cat ..\testcases\book\test9.tig & cat ast_dump.txt ) // SEM_ERROR, types of if-then-else differ
test14.tig          ( semanttest.exe ..\testcases\book\test14.tig ) // SEM_ERROR cannot compare record and array
test15.tig          ( semanttest.exe ..\testcases\book\test15.tig & cat ..\testcases\book\test15.tig & cat ast_dump.txt ) // OK
test21.tig          ( semanttest.exe ..\testcases\book\test21.tig ) // SEM_ERROR function without return type defined (procedure) returns value!!!


## for-loop

test12.tig

The low index and the high index have to be of type int.
It is not allowed to assign values to the iterator variable (I will ignore this limitation)

The returned type of a for-loop is the last type of the body.

```
/* valid for and let */

let
    var a:= 0
in 
    for i:=0 to 100 do (a:=a+1;())
end
```
## while-loop

test10.tig
test20.tig

## Functions

test4.tig,          ( semanttest.exe ..\testcases\book\test4.tig & cat ..\testcases\book\test4.tig & cat ast_dump.txt ) // OK
test6.tig,          ( semanttest.exe ..\testcases\book\test6.tig & cat ..\testcases\book\test6.tig & cat ast_dump.txt ) // OK
test7.tig,          ( semanttest.exe ..\testcases\book\test7.tig & cat ..\testcases\book\test7.tig & cat ast_dump.txt ) // OK
test18.tig,         ( semanttest.exe ..\testcases\book\test18.tig & cat ..\testcases\book\test18.tig & cat ast_dump.txt ) // SEM_ERRRO, use of undeclared function do_nothing2, mutually recursive declarations separated by variable declaration
test19.tig,         ( semanttest.exe ..\testcases\book\test19.tig & cat ..\testcases\book\test19.tig & cat ast_dump.txt ) // SEM_ERROR, variable a not defined
test34.tig,         ( semanttest.exe ..\testcases\book\test34.tig & cat ..\testcases\book\test34.tig & cat ast_dump.txt ) // SEM_ERROR, formals and actuals have different types
test35.tig,         ( semanttest.exe ..\testcases\book\test35.tig & cat ..\testcases\book\test35.tig & cat ast_dump.txt ) // SEM_ERROR, formals are more then actuals
test36.tig,         ( semanttest.exe ..\testcases\book\test36.tig & cat ..\testcases\book\test36.tig & cat ast_dump.txt ) // SEM_ERROR, formals are fewer then actuals
test39.tig,         ( semanttest.exe ..\testcases\book\test39.tig & cat ..\testcases\book\test39.tig & cat ast_dump.txt ) // SEM_ERROR, function with same name within the same batch
test40.tig          ( semanttest.exe ..\testcases\book\test40.tig & cat ..\testcases\book\test40.tig & cat ast_dump.txt ) // SEM_ERROR, procedure returns value


```
let
    function g(a:int):int = a
in
    //g(2)
    0
end
```


# break

The break keyword can only be used inside for-loops and while-loops.

As stated on page 525 of the book in the appendix:

Break: 
The break expression terminates evaluation of the nearest enclosing while-expression or for-expression. 
A break in procedure p cannot terminate a loop in procedure q, even if p is nested within q. 
A break that is not within a while or for is illegal

LEGAL:

12.tig              ( semanttest.exe ..\testcases\WMBao\Good\12.tig ) // OK
19.tig              ( semanttest.exe ..\testcases\WMBao\Good\19.tig ) // OK
61.tig              ( semanttest.exe ..\testcases\WMBao\Good\61.tig ) // OK

Illegal: 

58.tig              ( semanttest.exe ..\testcases\WMBao\Bad\58.tig ) // Error: A break is used outside a for or while loop
75.tig              ( semanttest.exe ..\testcases\WMBao\Bad\75.tig ) // Error: A break is used outside a for or while loop

```
/* semantics of nested break */

let function f () =
    let 
        function g (i : int) : int =
            if i = 3 then break
    in
        for i := 0 to 9 do
            (
                g (i); 
                print (chr (i + ord ("0")))
            )
    end
in
	f ()
end

```


# Mutually Recursive Types

Mutually recursive types are types that reference each other in a way that all references eventually form a loop!
The loop can be very small as in a node-type that references itself or a set of types that reference each other in a circle.
There are two approaches to the problem.


Mutually recursive types
mutually_recursive_types_with_and_keyword.tig   ( semanttest.exe ..\testcases\mutually_recursive_types_with_and_keyword.tig ) // SYNTAX ERROR since: "Type declarations separated by the keyword and form a mutually-recursive group." The "and" keyword between type declarations is not implemented! It is not part of the official language.
mutually_recursive_types.tig                    ( semanttest.exe ..\testcases\mutually_recursive_types.tig ) // OK
test16.tig,         ( semanttest.exe ..\testcases\book\test16.tig & cat ..\testcases\book\test16.tig & cat ast_dump.txt ) // <============== This currently semantically validates as OK although it should not. I just do not understand why this is relevant!
test5.tig,          ( semanttest.exe ..\testcases\book\test5.tig  & cat ..\testcases\book\test5.tig & cat ast_dump.txt ) // OK
test17.tig,         ( semanttest.exe ..\testcases\book\test17.tig & cat ..\testcases\book\test17.tig & cat ast_dump.txt ) // should throw an error since definition of recursive type is interrupted
test6.tig,          ( semanttest.exe ..\testcases\book\test6.tig  & cat ..\testcases\book\test6.tig & cat ast_dump.txt ) // OK
test7.tig,          ( semanttest.exe ..\testcases\book\test7.tig  & cat ..\testcases\book\test7.tig & cat ast_dump.txt ) // OK
test18.tig,         ( semanttest.exe ..\testcases\book\test18.tig & cat ..\testcases\book\test18.tig & cat ast_dump.txt ) // SEM_ERROR, mutually recursive definitions are interrupted by variable declaration
test19.tig,         ( semanttest.exe ..\testcases\book\test19.tig & cat ..\testcases\book\test19.tig & cat ast_dump.txt ) // SEM_ERROR, variable a not defined




## Approach 1 - Official Approach

The official way described in the book is to consider mutually recursive types
valid, when they are defined in a consecutive batch of declarations (See Appendix).

Here is the excerpt from the Appendix:

Mutually recursive types: A collection of types may be recursive or mutually recursive. 
Mutually recursive types are declared by a consecutive sequence of type declarations without 
intervening value or function declarations. 

Each recursion cycle must pass through a record or array type.

Thus, the type of lists of integers is legal:

```
type intlist = { hd: int, tl: intlist }
type tree = { key: int, children: treelist }
type treelist = { hd: tree, tl: treelist }
```

But the following declaration sequence is illegal:

```
type b = c
type c = b
```

To implement the official approach, follow the steps outlined in the book on page 114.

When processing mutually recursive types, we will need a place-holder for
types whose name we know but whose definition we have not yet seen. 

We can create a Ty_Name(sym, NULL) as a place-holder for the type-name sym
and later on fill in the ty field of the Ty_Name object with the type that sym
is supposed to stand for.



## Approach 2 - Approach from https://www.haber-kucharsky.com/proj/fang/tiger.html

The second way to treat mutually recursive types is usind a "and"-keyword as defined here: https://www.haber-kucharsky.com/proj/fang/tiger.html

1.4.4 Mutually-recursive types
Type declarations separated by the keyword "and" form a mutually-recursive group.

For example,

```
let
    type tree = {root: item, children: forest}
    and type forest = {head: tree, tail: forest}
    and type item = string

    function leaf(x: string): tree = tree {root=x, children=nil}
    function cons(x: tree, f: forest): forest = forest {head=x, tail=f}
in
    tree {root="Z", children=cons(leaf("A"), cons(leaf("B"), cons(leaf("C"), nil)))}
end
```

See mutually_recursive_types_with_and_keyword.tig


# Mutually Recursive Functions

From the book on page 99

The Tiger language treats adjacent function declarations as (possibly) mutually recursive. The FunctionDec constructor of the abstract syntax takes
a list of function declarations, not just a single function. The intent is that
this list is a maximal consecutive sequence of function declarations. Thus,
functions declared by the same FunctionDec can be mutually recursive.
Therefore, this program translates into the abstract syntax

## Tests for Mutually Recursive Functions

test6.tig           ( semanttest.exe ..\testcases\book\test6.tig ) // OK
test7.tig           ( semanttest.exe ..\testcases\book\test7.tig ) // OK
test18.tig          ( semanttest.exe ..\testcases\book\test18.tig ) // SEM_ERROR, variable declaration interrupts mutually recursive type declarations
test19.tig          ( semanttest.exe ..\testcases\book\test19.tig ) // SEM_ERROR, var a undeclared




# Tests

Atomic Expressions:
vardec_no_type_and_initializer.tig      ( semanttest.exe ..\testcases\vardec_no_type_and_initializer.tig & cat ast_dump.txt ) OK
vardec_no_type_no_initializer.tig       ( semanttest.exe ..\testcases\vardec_no_type_no_initializer.tig ) SYNTAX_ERROR
vardec_type_no_initialization.tig       ( semanttest.exe ..\testcases\vardec_type_no_initialization.tig ) SYNTAX_ERROR

Assignments:
assignment.tig      ( semanttest.exe ..\testcases\assignment.tig    & cat ast_dump.txt ) // SEMANTIC_ERROR (no type specified for variable i)

Sequences:
sequencing.tig      ( semanttest.exe ..\testcases\sequencing.tig    & cat ast_dump.txt ) // SEMANTIC_ERROR, i has no type

Let
let.tig             ( semanttest.exe ..\testcases\let.tig           & cat ..\testcases\let.tig           & cat ast_dump.txt ) // SEM-ERROR
let_nested.tig      ( semanttest.exe ..\testcases\let_nested.tig    & cat ..\testcases\let_nested.tig    & cat ast_dump.txt ) // OK

Operators
addition.tig        ( semanttest.exe ..\testcases\addition.tig      & cat ..\testcases\addition.tig      & cat ast_dump.txt ) // OK
opers.tig           ( semanttest.exe ..\testcases\opers.tig         & cat ..\testcases\opers.tig         & cat ast_dump.txt ) // OK

Assignment
assignment.tig      ( semanttest.exe ..\testcases\assignment.tig    & cat ..\testcases\assignment.tig    & cat ast_dump.txt ) // SEM-ERROR, variable i not declared

Arrays
arrays.tig          ( semanttest.exe ..\testcases\arrays.tig        & cat ..\testcases\arrays.tig        & cat ast_dump.txt ) // OK
arrays_simple.tig   ( semanttest.exe ..\testcases\arrays_simple.tig & cat ..\testcases\arrays_simple.tig & cat ast_dump.txt ) // OK

Built-In functions
builtin_functions.tig           ( semanttest.exe ..\testcases\builtin_functions.tig & cat ..\testcases\builtin_functions.tig & cat ast_dump.txt ) // OK
builtin_functions_getchar.tig   ( semanttest.exe ..\testcases\builtin_functions_getchar.tig ) OK

For-Loop
forloop_simple.tig                  ( semanttest.exe ..\testcases\forloop_simple.tig ) // SEM-ERROR, hi is not of type int
forloop_non_declared_variable.tig   ( semanttest.exe ..\testcases\forloop_non_declared_variable.tig ) // OK

Functions
( semanttest.exe ..\testcases\function_complex_2.tig ) // OK
( semanttest.exe ..\testcases\function_complex_3.tig ) // OK
( semanttest.exe ..\testcases\function_complex.tig ) // OK
( semanttest.exe ..\testcases\function_simple.tig ) // OK
( semanttest.exe ..\testcases\function_use_parameter_confusing.tig ) // OK
( semanttest.exe ..\testcases\function_use_parameter.tig ) // OK
( semanttest.exe ..\testcases\function.tig ) // OK

if-then
if-then-else
( semanttest.exe ..\testcases\if_nil.tig ) // OK
( semanttest.exe ..\testcases\if_sequence.tig ) // OK
( semanttest.exe ..\testcases\if_simple.tig ) // SEM-ERROR, variable i not declared

let
( semanttest.exe ..\testcases\let_nested.tig ) // OK
( semanttest.exe ..\testcases\let.tig ) // SEM-ERROR semantic error (types do not match)

nil
( semanttest.exe ..\testcases\nil_matches_any_type.tig ) // OK

records
( semanttest.exe ..\testcases\records_field_assignment.tig ) // OK
( semanttest.exe ..\testcases\records_simple.tig ) // OK
( semanttest.exe ..\testcases\records_var_declaration.tig ) // OK

Sequences
( semanttest.exe ..\testcases\sequencing.tig ) // SEM-ERROR, variable i not declared

Type Declarations
( semanttest.exe ..\testcases\typedec_simple.tig ) // OK

Variable Declarations
( semanttest.exe ..\testcases\vardec_nil.tig ) // OK
( semanttest.exe ..\testcases\vardec_no_type_and_initializer.tig ) // OK
( semanttest.exe ..\testcases\vardec_no_type_no_initializer.tig ) // SYNTAX ERROR, initializer missing
( semanttest.exe ..\testcases\vardec_type_no_initialization.tig ) // SYNTAX ERROR, initializer missing

Applications
fact.tig            ( semanttest.exe ..\testcases\fact.tig ) // OK
merge.tig           ( semanttest.exe ..\testcases\merge.tig ) // OK
queens.tig          ( semanttest.exe ..\testcases\queens.tig ) // OK

Mutually recursive types
mutually_recursive_types_with_and_keyword.tig   ( semanttest.exe ..\testcases\mutually_recursive_types_with_and_keyword.tig ) // SYNTAX ERROR since: "Type declarations separated by the keyword and form a mutually-recursive group." The "and" keyword between type declarations is not implemented! It is not part of the official language.
mutually_recursive_types.tig                    ( semanttest.exe ..\testcases\mutually_recursive_types.tig ) // OK
test16.tig,         ( semanttest.exe ..\testcases\book\test16.tig & cat ..\testcases\book\test16.tig & cat ast_dump.txt ) // <============== This currently semantically validates as OK although it should not. I do not understand why this code is a problem!
test5.tig,          ( semanttest.exe ..\testcases\book\test5.tig  & cat ..\testcases\book\test5.tig & cat ast_dump.txt ) // OK
test17.tig,         ( semanttest.exe ..\testcases\book\test17.tig & cat ..\testcases\book\test17.tig & cat ast_dump.txt ) // should throw an error since definition of recursive type is interrupted
test6.tig,          ( semanttest.exe ..\testcases\book\test6.tig  & cat ..\testcases\book\test6.tig & cat ast_dump.txt ) // OK
test7.tig,          ( semanttest.exe ..\testcases\book\test7.tig  & cat ..\testcases\book\test7.tig & cat ast_dump.txt ) // OK
test18.tig,         ( semanttest.exe ..\testcases\book\test18.tig & cat ..\testcases\book\test18.tig & cat ast_dump.txt ) // SEM_ERROR, mutually recursive function declarations are interrupted by variable declaration
test19.tig,         ( semanttest.exe ..\testcases\book\test19.tig & cat ..\testcases\book\test19.tig & cat ast_dump.txt ) // SEM_ERROR, a not declared

mutually recursive functions
test6.tig           ( semanttest.exe ..\testcases\book\test6.tig ) // OK
test7.tig           ( semanttest.exe ..\testcases\book\test7.tig ) // OK
test18.tig          ( semanttest.exe ..\testcases\book\test18.tig ) // SEM_ERROR, mutually recursive function declarations are interrupted by variable declaration
test19.tig          ( semanttest.exe ..\testcases\book\test19.tig ) // SEM_ERROR, a not declared


All the following tests are in the testcases\book folder

Comparison Operators:
test13.tig          ( semanttest.exe ..\testcases\book\test13.tig & cat ast_dump.txt ) // SEMANTIC ERROR

Arithmetic Operators:
test26.tig          ( semanttest.exe ..\testcases\book\test26.tig & cat ..\testcases\book\test26.tig & cat ast_dump.txt ) // SEM_ERROR, integer required

if-then-else:
test8.tig,          ( semanttest.exe ..\testcases\book\test8.tig  & cat ast_dump.txt ) // OK
test9.tig,          ( semanttest.exe ..\testcases\book\test9.tig  & cat ..\testcases\book\test9.tig & cat ast_dump.txt ) // SEM_ERROR: types differ
test15.tig          ( semanttest.exe ..\testcases\book\test15.tig & cat ..\testcases\book\test15.tig & cat ast_dump.txt ) // SEM_ERROR: this application returns a unit (Non-Type) since the if-then does not return a type! Returning unit is not allowed. You need a sequence or something to produce a type to return!

while-loops:
test10.tig          ( semanttest.exe ..\testcases\book\test10.tig & cat ..\testcases\book\test10.tig & cat ast_dump.txt ) // OK

for-loops:
test11.tig          ( semanttest.exe ..\testcases\book\test11.tig & cat ..\testcases\book\test11.tig & cat ast_dump.txt ) // SEM_ERROR hi has to be of type integer

sequences:
test20.tig          ( semanttest.exe ..\testcases\book\test20.tig & cat ..\testcases\book\test20.tig & cat ast_dump.txt ) // SEM_ERROR undeclared variable

array-indexing:
test24.tig          ( semanttest.exe ..\testcases\book\test24.tig & cat ..\testcases\book\test24.tig & cat ast_dump.txt ) // SEM_ERROR Subscript used on variable "d". Variable is not an array! Line: 6
test32.tig          ( semanttest.exe ..\testcases\book\test32.tig & cat ..\testcases\book\test32.tig & cat ast_dump.txt ) // SEM_ERROR initializing exp and array type differ

record field acces:
test25.tig          ( semanttest.exe ..\testcases\book\test25.tig & cat ..\testcases\book\test25.tig & cat ast_dump.txt ) // SEM_ERROR

variable declarations:
test12.tig,         ( semanttest.exe ..\testcases\book\test12.tig & cat ..\testcases\book\test12.tig & cat ast_dump.txt ) // OK
test31.tig,         ( semanttest.exe ..\testcases\book\test31.tig & cat ..\testcases\book\test31.tig & cat ast_dump.txt ) // SEM_ERROR, initializer type and variable type differ!
test37.tig,         ( semanttest.exe ..\testcases\book\test37.tig & cat ..\testcases\book\test37.tig & cat ast_dump.txt ) // OK
test41.tig,         ( semanttest.exe ..\testcases\book\test41.tig & cat ..\testcases\book\test41.tig & cat ast_dump.txt ) // OK
test42.tig,         ( semanttest.exe ..\testcases\book\test42.tig & cat ..\testcases\book\test42.tig & cat ast_dump.txt ) // OK
test43.tig          ( semanttest.exe ..\testcases\book\test43.tig & cat ..\testcases\book\test43.tig & cat ast_dump.txt ) // SEM_ERRRO, unit type of variable causes semant to signal an error later when variable is used

record type definitions:
test33.tig,         ( semanttest.exe ..\testcases\book\test33.tig & cat ..\testcases\book\test33.tig & cat ast_dump.txt ) // SEM_ERROR, undeclared type
test44.tig,         ( semanttest.exe ..\testcases\book\test44.tig & cat ..\testcases\book\test44.tig & cat ast_dump.txt ) // SEM_ERROR
test45.tig,         ( semanttest.exe ..\testcases\book\test45.tig & cat ..\testcases\book\test45.tig & cat ast_dump.txt ) // SEM_ERROR, unit return
test46.tig,         ( semanttest.exe ..\testcases\book\test46.tig & cat ..\testcases\book\test46.tig & cat ast_dump.txt ) // OK
test49.tig          ( semanttest.exe ..\testcases\book\test49.tig & cat ..\testcases\book\test49.tig & cat ast_dump.txt ) // SYNTAX ERROR

type definitions:
test1.tig,          ( semanttest.exe ..\testcases\book\test1.tig  & cat ..\testcases\book\test1.tig & cat ast_dump.txt ) // OK
test2.tig,          ( semanttest.exe ..\testcases\book\test2.tig  & cat ..\testcases\book\test2.tig & cat ast_dump.txt ) // OK
test3.tig,          ( semanttest.exe ..\testcases\book\test3.tig  & cat ..\testcases\book\test3.tig & cat ast_dump.txt ) // OK
test14.tig,         ( semanttest.exe ..\testcases\book\test14.tig & cat ..\testcases\book\test14.tig & cat ast_dump.txt ) // SEM-ERROR, types do not match
test22.tig,         ( semanttest.exe ..\testcases\book\test22.tig & cat ..\testcases\book\test22.tig & cat ast_dump.txt ) // SEM-ERRRO, field not in record type
test23.tig,         ( semanttest.exe ..\testcases\book\test23.tig & cat ..\testcases\book\test23.tig & cat ast_dump.txt ) // SEM-ERROR, type mismatch
test28.tig,         ( semanttest.exe ..\testcases\book\test28.tig & cat ..\testcases\book\test28.tig & cat ast_dump.txt ) // SEM-ERROR, different record types
test29.tig,         ( semanttest.exe ..\testcases\book\test29.tig & cat ..\testcases\book\test29.tig & cat ast_dump.txt ) // SEM-ERROR, different array types
test30.tig,         ( semanttest.exe ..\testcases\book\test30.tig & cat ..\testcases\book\test30.tig & cat ast_dump.txt ) // OK
test38.tig,         ( semanttest.exe ..\testcases\book\test38.tig & cat ..\testcases\book\test38.tig & cat ast_dump.txt ) // SEM-ERROR, two types of same name in the same batch
test47.tig,         ( semanttest.exe ..\testcases\book\test47.tig & cat ..\testcases\book\test47.tig & cat ast_dump.txt ) // OK
test48.tig          ( semanttest.exe ..\testcases\book\test48.tig & cat ..\testcases\book\test48.tig & cat ast_dump.txt ) // OK

function declarations and function calls:
test4.tig,          ( semanttest.exe ..\testcases\book\test4.tig  & cat ..\testcases\book\test4.tig & cat ast_dump.txt ) // OK
test6.tig,          ( semanttest.exe ..\testcases\book\test6.tig  & cat ..\testcases\book\test6.tig & cat ast_dump.txt ) // OK
test7.tig,          ( semanttest.exe ..\testcases\book\test7.tig  & cat ..\testcases\book\test7.tig & cat ast_dump.txt ) // OK
test18.tig,         ( semanttest.exe ..\testcases\book\test18.tig & cat ..\testcases\book\test18.tig & cat ast_dump.txt ) // OK
test19.tig,         ( semanttest.exe ..\testcases\book\test19.tig & cat ..\testcases\book\test19.tig & cat ast_dump.txt ) // SEM-ERRIR, var a not defined
test34.tig,         ( semanttest.exe ..\testcases\book\test34.tig & cat ..\testcases\book\test34.tig & cat ast_dump.txt ) // SEM-ERROR, formal and actual parameter types differ
test35.tig,         ( semanttest.exe ..\testcases\book\test35.tig & cat ..\testcases\book\test35.tig & cat ast_dump.txt ) // SEM-ERROR, formals are more then actuals
test36.tig,         ( semanttest.exe ..\testcases\book\test36.tig & cat ..\testcases\book\test36.tig & cat ast_dump.txt ) // SEM-ERROR, formals are fewer then actuals
test39.tig,         ( semanttest.exe ..\testcases\book\test39.tig & cat ..\testcases\book\test39.tig & cat ast_dump.txt ) // OK, in this implementation
test40.tig          ( semanttest.exe ..\testcases\book\test40.tig & cat ..\testcases\book\test40.tig & cat ast_dump.txt ) // SEM-ERROR, no return type declared but returns ty_int

function calls:
test21.tig,         ( semanttest.exe ..\testcases\book\test21.tig & cat ..\testcases\book\test21.tig & cat ast_dump.txt ) // SEM-ERROR, procedure (= no return value, returns unit) is used where return type is required
test27.tig          ( semanttest.exe ..\testcases\book\test27.tig & cat ..\testcases\book\test27.tig & cat ast_dump.txt ) // OK


break:
two_breaks.tig      ( semanttest.exe ..\testcases\BartVandewoestyne\uncompilable\two_breaks.tig ) // SEM_ERROR, type of operand has to be int
breaktest.tig       ( semanttest.exe ..\testcases\FlexW\breaktest.tig ) // SEM_ERROR, break outside loop
keyword_04.tig      ( semanttest.exe ..\testcases\nwtnni\lex\keyword_04.tig ) // SEM_ERROR, break outside loop
58.tig              ( semanttest.exe ..\testcases\WMBao\Bad\58.tig ) // SEM-ERROR, A break is used outside a for or while loop
75.tig              ( semanttest.exe ..\testcases\WMBao\Bad\75.tig ) // SEM-ERROR, A break is used outside a for or while loop
12.tig              ( semanttest.exe ..\testcases\WMBao\Good\12.tig ) // OK
19.tig              ( semanttest.exe ..\testcases\WMBao\Good\19.tig ) // OK
61.tig              ( semanttest.exe ..\testcases\WMBao\Good\61.tig ) // OK


Shadowing variables

Shadowing types
test47.tig          ( semanttest.exe ..\testcases\book\test47.tig ) // OK, Shadowing is allowed! Two types both called 'a' are created in the current tenv but this is fine if the uppermost type from the tenv is used! The uppermost type is the type defined last which shadows all other types!



semanttest.exe ..\testcases\BartVandewoestyne\compilable\array_equality.tig // OK
semanttest.exe ..\testcases\BartVandewoestyne\compilable\control_characters.tig // OK
semanttest.exe ..\testcases\BartVandewoestyne\compilable\escape_sequences.tig // OK
semanttest.exe ..\testcases\BartVandewoestyne\compilable\function.tig // SEM_ERROR, function call without required actual parameter
semanttest.exe ..\testcases\BartVandewoestyne\compilable\negative_int.tig // OK
semanttest.exe ..\testcases\BartVandewoestyne\compilable\procedure.tig // OK
semanttest.exe ..\testcases\BartVandewoestyne\compilable\record_equality_test.tig // OK
semanttest.exe ..\testcases\BartVandewoestyne\compilable\simple_let.tig // OK
semanttest.exe ..\testcases\BartVandewoestyne\compilable\valid_strings.tig // OK

semanttest.exe ..\testcases\WMBao\Good\1.tig // OK
semanttest.exe ..\testcases\WMBao\Good\2.tig // OK
semanttest.exe ..\testcases\WMBao\Good\3.tig // OK
semanttest.exe ..\testcases\WMBao\Good\5.tig // OK
semanttest.exe ..\testcases\WMBao\Good\6.tig // OK
semanttest.exe ..\testcases\WMBao\Good\7.tig // OK
semanttest.exe ..\testcases\WMBao\Good\8.tig // OK
semanttest.exe ..\testcases\WMBao\Good\10.tig // SYNTAX ERROR
semanttest.exe ..\testcases\WMBao\Good\11.tig // OK
semanttest.exe ..\testcases\WMBao\Good\12.tig // OK
semanttest.exe ..\testcases\WMBao\Good\13.tig // OK
semanttest.exe ..\testcases\WMBao\Good\14.tig // OK
semanttest.exe ..\testcases\WMBao\Good\15.tig // OK
semanttest.exe ..\testcases\WMBao\Good\16.tig // OK
semanttest.exe ..\testcases\WMBao\Good\17.tig // OK
semanttest.exe ..\testcases\WMBao\Good\18.tig // SEM_WARNING, printi() not defined!
semanttest.exe ..\testcases\WMBao\Good\19.tig // OK
semanttest.exe ..\testcases\WMBao\Good\20.tig // SEM_WARNING, printi() not defined!
semanttest.exe ..\testcases\WMBao\Good\21.tig // SEM_WARNING, printi() not defined!
semanttest.exe ..\testcases\WMBao\Good\22.tig // OK
semanttest.exe ..\testcases\WMBao\Good\23.tig // OK, this is the queens.tig by appel
semanttest.exe ..\testcases\WMBao\Good\24.tig // SEM_WARNING, printi() not defined!
semanttest.exe ..\testcases\WMBao\Good\25.tig // OK
semanttest.exe ..\testcases\WMBao\Good\26.tig // OK
semanttest.exe ..\testcases\WMBao\Good\27.tig // SEM_WARNING, printi() not defined!
semanttest.exe ..\testcases\WMBao\Good\28.tig // SEM_WARNING, printi() not defined!
semanttest.exe ..\testcases\WMBao\Good\29.tig // OK
semanttest.exe ..\testcases\WMBao\Good\30.tig // OK
semanttest.exe ..\testcases\WMBao\Good\32.tig // OK
semanttest.exe ..\testcases\WMBao\Good\33.tig // OK
semanttest.exe ..\testcases\WMBao\Good\34.tig // OK
semanttest.exe ..\testcases\WMBao\Good\35.tig // OK
semanttest.exe ..\testcases\WMBao\Good\36.tig // SEM_WARNING: use of undeclared function printi
semanttest.exe ..\testcases\WMBao\Good\38.tig // SEM_WARNING: use of undeclared function printi
semanttest.exe ..\testcases\WMBao\Good\40.tig // OK
semanttest.exe ..\testcases\WMBao\Good\43.tig // OK
semanttest.exe ..\testcases\WMBao\Good\52.tig // OK
semanttest.exe ..\testcases\WMBao\Good\61.tig // OK
semanttest.exe ..\testcases\WMBao\Good\68.tig // OK
semanttest.exe ..\testcases\WMBao\Good\69.tig // OK
semanttest.exe ..\testcases\WMBao\Good\70.tig // OK
semanttest.exe ..\testcases\WMBao\Good\71.tig // OK
semanttest.exe ..\testcases\WMBao\Good\72.tig // OK
semanttest.exe ..\testcases\WMBao\Good\73.tig // OK
semanttest.exe ..\testcases\WMBao\Good\74.tig // OK
semanttest.exe ..\testcases\WMBao\Good\76.tig // OK
semanttest.exe ..\testcases\WMBao\Good\77.tig // SEM_WARNING: use of undeclared function printi
semanttest.exe ..\testcases\WMBao\Good\78.tig // OK
semanttest.exe ..\testcases\WMBao\Good\79.tig // OK
semanttest.exe ..\testcases\WMBao\Good\80.tig // SEM_WARNING: use of undeclared function printi
semanttest.exe ..\testcases\WMBao\Good\81.tig // SEM_WARNING: use of undeclared function printi
semanttest.exe ..\testcases\WMBao\Good\82.tig // SEM_WARNING: use of undeclared function printi
semanttest.exe ..\testcases\WMBao\Good\83.tig
semanttest.exe ..\testcases\WMBao\Good\84.tig // SEM_WARNING: use of undeclared function printi
semanttest.exe ..\testcases\WMBao\Good\85.tig // SEM_WARNING: use of undeclared function printi
semanttest.exe ..\testcases\WMBao\Good\86.tig // OK
semanttest.exe ..\testcases\WMBao\Good\87.tig // OK
semanttest.exe ..\testcases\WMBao\Good\88.tig // OK
semanttest.exe ..\testcases\WMBao\Good\pi.tig // SEM_WARNING: use of undeclared function printi


The following tests in the bad folder only make sense when the parser has been extended to understand
the boolean datatype

semanttest.exe ..\testcases\WMBao\Bad\1.tig // SEM_ERROR: variable "true" not defined. Tiger officially has no boolean datatype
semanttest.exe ..\testcases\WMBao\Bad\2.tig // SEM_ERROR: variable "false" not defined. Tiger officially has no boolean datatype
semanttest.exe ..\testcases\WMBao\Bad\3.tig // SEM_ERROR: integer required for operator
semanttest.exe ..\testcases\WMBao\Bad\4.tig // SEM_ERROR: variable "true" not defined. Tiger officially has no boolean datatype
semanttest.exe ..\testcases\WMBao\Bad\5.tig
semanttest.exe ..\testcases\WMBao\Bad\6.tig
semanttest.exe ..\testcases\WMBao\Bad\7.tig
semanttest.exe ..\testcases\WMBao\Bad\8.tig
semanttest.exe ..\testcases\WMBao\Bad\9.tig
semanttest.exe ..\testcases\WMBao\Bad\10.tig // SEM_ERROR: if-then-else invalid! then and else are of different type!
semanttest.exe ..\testcases\WMBao\Bad\11.tig
semanttest.exe ..\testcases\WMBao\Bad\12.tig
semanttest.exe ..\testcases\WMBao\Bad\13.tig
semanttest.exe ..\testcases\WMBao\Bad\15.tig
semanttest.exe ..\testcases\WMBao\Bad\18.tig
semanttest.exe ..\testcases\WMBao\Bad\19.tig // SEM_ERROR: structs are not the same type
semanttest.exe ..\testcases\WMBao\Bad\20.tig
semanttest.exe ..\testcases\WMBao\Bad\23.tig
semanttest.exe ..\testcases\WMBao\Bad\24.tig
semanttest.exe ..\testcases\WMBao\Bad\25.tig
semanttest.exe ..\testcases\WMBao\Bad\26.tig
semanttest.exe ..\testcases\WMBao\Bad\29.tig // why is nil = nil bad?
semanttest.exe ..\testcases\WMBao\Bad\32.tig // SEM_ERROR
semanttest.exe ..\testcases\WMBao\Bad\41.tig
semanttest.exe ..\testcases\WMBao\Bad\42.tig
semanttest.exe ..\testcases\WMBao\Bad\43.tig
semanttest.exe ..\testcases\WMBao\Bad\46.tig
semanttest.exe ..\testcases\WMBao\Bad\47.tig
semanttest.exe ..\testcases\WMBao\Bad\48.tig // SEM_ERROR types are different (see also semanttest.exe ..\testcases\book\test29.tig)
semanttest.exe ..\testcases\WMBao\Bad\58.tig
semanttest.exe ..\testcases\WMBao\Bad\59.tig
semanttest.exe ..\testcases\WMBao\Bad\60.tig
semanttest.exe ..\testcases\WMBao\Bad\62.tig
semanttest.exe ..\testcases\WMBao\Bad\75.tig