/****************************************************/
/* File: symtab.h                                   */
/* Symbol table interface for the TINY compiler     */
/* (allows only one symbol table)                   */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "globals.h"
#define SIZE 211

typedef enum {Func, Var, Arg} SymKind; 

typedef struct LineListRec
   { int lineno;
     struct LineListRec * next;
   } * LineList;

typedef struct BucketListRec
   { char * name;
     ExpType type;
     SymKind kind;
     LineList lines;
     int memloc;
     struct BucketListRec * next;
   } * BucketList;

typedef struct ScopeListRec
   { char * name;
     BucketList hashTable[SIZE];
     struct ScopeListRec * parent;
     int depth;
   } * ScopeList;

typedef struct ArgListRec
   { ExpType type;
     char * name;
     struct ArgListRec * next;
   } * ArgList;

typedef struct FuncListRec
   { ExpType type;
     char * name;
     ArgList args;
   } * FuncList;

void st_init( void );

void st_newScope( char * name );

void st_push(ScopeList s);

void st_pop(void);

ScopeList st_get_top(void);

int addLoc(void);

void popLoc(void);

void addLine(char * name, int line);

int st_getSize(void);

ScopeList st_getTable(int index);

ScopeList st_getFunc(char * name);

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( char * name, ExpType type, SymKind kind, int lineno, int loc );

/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */
BucketList st_lookup (char * name);

BucketList st_lookup_excluding_parent (ScopeList s, char * name);

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printAllSymTab(FILE * listing);

#endif
