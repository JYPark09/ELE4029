/*******************************************************/
/* File: symtab.c                                      */
/* Symbol table implementation for the C-MINUS compiler*/
/* (allows only one symbol table)                      */
/* Symbol table is implemented as a chained            */
/* hash table                                          */
/* Compiler Construction: Principles and Practice      */
/* Kenneth C. Louden                                   */
/*******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"
#include "util.h"

/* SHIFT is the power of two used as multiplier
   in hash function  */
#define SHIFT 4

/* the hash function */
static int hash ( char * key )
{ int temp = 0;
  int i = 0;
  while (key[i] != '\0')
  { temp = ((temp << SHIFT) + key[i]) % HASH_TBL_SIZE;
    ++i;
  }
  return temp;
}

/* the hash table */
ScopeList globalScope;

ScopeList buildScope(char * name, ScopeList parent)
{
  ScopeList newScope = malloc(sizeof(struct ScopeListRec));
  newScope->name = copyString(name);
  newScope->parent = parent;
  newScope->leftMostChild = NULL;
  newScope->rightSibling = NULL;
  for (int i = 0; i < HASH_TBL_SIZE; i++)
  {
    newScope->bucket[i] = NULL;
  }
  return newScope;
}

void addChildScope(ScopeList parent, ScopeList child)
{
  if (parent->leftMostChild == NULL)
  {
    parent->leftMostChild = child;
  }
  else
  {
    ScopeList current = parent->leftMostChild;
    while (current->rightSibling != NULL)
    {
      current = current->rightSibling;
    }
    current->rightSibling = child;
  }
}

ScopeList findScope(char * name, ScopeList parent)
{
  ScopeList current = parent->leftMostChild;

  while (current != NULL)
  {
    if (strcmp(current->name, name) == 0)
    {
      return current;
    }
    current = current->rightSibling;
  }

  return NULL;
}

void init_symtab()
{
  globalScope = buildScope("global", NULL);

  // built-in functions
  BucketList output_bl = st_insert(globalScope, "output", Function, 0, 1);
  output_bl->func.type = Void;
  output_bl->func.params = 1;
  output_bl->func.param[0].name = copyString("value");
  output_bl->func.param[0].type = Integer;

  BucketList input_bl = st_insert(globalScope, "input", Function, 0, 0);
  input_bl->func.type = Integer;
  input_bl->func.params = 0;
}

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
BucketList st_insert( ScopeList scope, char * name, ExpType type, int lineno, int loc )
{ int h = hash(name);
  BucketList l =  scope->bucket[h];
  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;
  if (l == NULL) /* variable not yet in table */
  { l = (BucketList) malloc(sizeof(struct BucketListRec));
    l->name = name;
    l->type = type;
    l->lines = (LineList) malloc(sizeof(struct LineListRec));
    l->lines->lineno = lineno;
    l->memloc = loc;
    l->lines->next = NULL;
    l->next = scope->bucket[h];
    l->scope = scope;
    scope->bucket[h] = l; }
  else /* found in table, so just add line number */
  { LineList t = l->lines;
    while (t->next != NULL) t = t->next;
    t->next = (LineList) malloc(sizeof(struct LineListRec));
    t->next->lineno = lineno;
    t->next->next = NULL;
  }
  return l;
} /* st_insert */

/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */
BucketList st_lookup ( ScopeList scope, char * name )
{ int h = hash(name);

  while (scope != NULL)
  {
    BucketList l =  scope->bucket[h];
    while ((l != NULL) && (strcmp(name,l->name) != 0))
      l = l->next;
    if (l != NULL) return l;

    scope = scope->parent;
  }

  return NULL;
}

static void print_traverse( ScopeList t, FILE * listing,
               void (* proc) (ScopeList, FILE*) )
{ if (t != NULL)
  { proc(t, listing);
    if (t->leftMostChild != NULL)
      print_traverse(t->leftMostChild,listing,proc);
    print_traverse(t->rightSibling,listing,proc);
  }
}

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
const char* typestr(ExpType t)
{
  switch (t)
  { case Void: return "void";
    case Integer: return "int";
    case VoidArr: return "void[]";
    case IntegerArr: return "int[]";
    case Boolean: return "bool";
    case Function: return "Function";
    default: return "Unknown";
  }
}

void printScope(ScopeList scope, FILE * listing)
{ int i;
  for (i=0;i<HASH_TBL_SIZE;++i)
  { if (scope->bucket[i] != NULL)
    { BucketList l = scope->bucket[i];
      while (l != NULL)
      { LineList t = l->lines;
        fprintf(listing,"%-14s ",l->name);
        fprintf(listing,"%-14s ",typestr(l->type));
        fprintf(listing,"%-11s ",scope->name);
        fprintf(listing,"%-8d  ",l->memloc);
        while (t != NULL)
        { fprintf(listing,"%4d ",t->lineno);
          t = t->next;
        }
        fprintf(listing,"\n");
        l = l->next;
      }
    }
  }
}

void printFunc(ScopeList scope, FILE * listing)
{ int i, p;
  for (i=0;i<HASH_TBL_SIZE;++i)
  { if (scope->bucket[i] != NULL)
    { BucketList l = scope->bucket[i];
      while (l != NULL)
      { if (l->type == Function) {
          fprintf(listing,"%-14s ",l->name);
          fprintf(listing,"%-11s ",scope->name);
          fprintf(listing,"%-12s ",typestr(l->func.type));
          if (l->func.params == 0)
          {
            fprintf(listing,"                ");
            fprintf(listing,"%-14s", "Void");
          }
          else
          {
            for (p=0;p<l->func.params;++p)
            { fprintf(listing,"\n                                        ");
              fprintf(listing,"%-15s ",l->func.param[p].name);
              fprintf(listing,"%-14s",typestr(l->func.param[p].type));
            }
          }
          fprintf(listing,"\n");
        }
        l = l->next;
      }
    }
  }
}

void printWithLevel(ScopeList scope, FILE * listing)
{ int i, level;
  ScopeList tmpScope = scope;

  if (scope == globalScope) return;

  for (level=0;tmpScope->parent != NULL;++level)
    tmpScope = tmpScope->parent;

  for (i=0;i<HASH_TBL_SIZE;++i)
  { if (scope->bucket[i] != NULL)
    { BucketList l = scope->bucket[i];
      while (l != NULL)
      { if (l->type != Function) {
          fprintf(listing,"%-15s ",scope->name);
          fprintf(listing,"%-13d ",level);
          fprintf(listing,"%-14s ",l->name);
          fprintf(listing,"%-11s",typestr(l->type));
          fprintf(listing,"\n");
        }
        l = l->next;
      }
    }
  }
}

void printSymTab(FILE * listing)
{
  fprintf(listing,"Variable Name  Variable Type  Scope Name  Location   Line Numbers\n");
  fprintf(listing,"-------------  -------------  ----------  --------   ------------\n");
  print_traverse(globalScope, listing, printScope);
} /* printSymTab */

void printFuncTab(FILE * listing)
{
  fprintf(listing,"Function Name  Scope Name  Return Type  Parameter Name  Parameter Type\n");
  fprintf(listing,"-------------  ----------  -----------  --------------  --------------\n");
  print_traverse(globalScope, listing, printFunc);
}

void printFuncAndGlobalTab(FILE * listing)
{ int i;
  fprintf(listing,"   ID Name     ID Type    Data Type\n");
  fprintf(listing,"------------  ---------  -----------\n");

  for (i=0;i<HASH_TBL_SIZE;++i)
  { if (globalScope->bucket[i] != NULL)
    { BucketList l = globalScope->bucket[i];
      while (l != NULL)
      {
        fprintf(listing,"%-13s ",l->name);
        fprintf(listing,"%-10s ",(l->type == Function) ? "Function" : "Variable");
        fprintf(listing,"%-11s ",typestr((l->type == Function) ? l->func.type : l->type));
        fprintf(listing,"\n");
        l = l->next;
      }
    }
  }
}

void printLocalVarTab(FILE * listing)
{
  fprintf(listing,"  Scope Name    Nested Level     ID Name      Data Type\n");
  fprintf(listing,"--------------  ------------  -------------  -----------\n");
  print_traverse(globalScope, listing, printWithLevel);
}
