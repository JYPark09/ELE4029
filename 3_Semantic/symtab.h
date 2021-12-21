/****************************************************/
/* File: symtab.h                                   */
/* Symbol table interface for the C-MINUS compile   */
/* (allows only one symbol table)                   */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "globals.h"

/* SIZE is the size of the hash table */
#define HASH_TBL_SIZE 211

#define MAX_FUNC_PARAMS 127

/* the list of line numbers of the source 
 * code in which a variable is referenced
 */
typedef struct LineListRec
   { int lineno;
     struct LineListRec * next;
   } * LineList;

/* The record in the bucket lists for
 * each variable, including name, 
 * assigned memory location, and
 * the list of line numbers in which
 * it appears in the source code
 */
typedef struct BucketListRec
   { char * name;
     ExpType type;
     LineList lines;
     int memloc ; /* memory location for variable */
     struct {
       ExpType type;
       int params;
       struct {
        char * name;
        ExpType type;
       } param[MAX_FUNC_PARAMS];
     } func;
     struct BucketListRec * next;
     struct ScopeListRec * scope;
   } * BucketList;

typedef struct ScopeListRec
   { char * name;
     BucketList bucket[HASH_TBL_SIZE];
     struct ScopeListRec * parent;
     struct ScopeListRec * leftMostChild;
     struct ScopeListRec * rightSibling;
   } * ScopeList;

ScopeList buildScope(char * name, ScopeList parent);
void addChildScope(ScopeList parent, ScopeList child);
ScopeList findScope(char * name, ScopeList parent);

void init_symtab();

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
BucketList st_insert( ScopeList scope, char * name, ExpType type, int lineno, int loc );

/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */
BucketList st_lookup ( ScopeList scope, char * name );

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE * listing);
void printFuncTab(FILE * listing);
void printFuncAndGlobalTab(FILE * listing);
void printLocalVarTab(FILE * listing);

#endif
