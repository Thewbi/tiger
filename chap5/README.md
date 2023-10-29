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

# Testing

```
semanttest.exe ..\testcases\addition.tig
```