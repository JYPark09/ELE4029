/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the C-MINUS compiler                         */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"
#include "util.h"

#include <stdarg.h>

/* counter for variable memory locations */
static int location = 0;

/* Procedure traverse is a generic recursive 
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc 
 * in postorder to tree pointed to by t
 */
static void traverse( TreeNode * t,
               void (* preProc) (TreeNode *),
               void (* postProc) (TreeNode *) )
{ if (t != NULL)
  { preProc(t);
    { int i;
      for (i=0; i < MAXCHILDREN; i++)
        traverse(t->child[i],preProc,postProc);
    }
    postProc(t);
    traverse(t->sibling,preProc,postProc);
  }
}

/* nullProc is a do-nothing procedure to 
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(TreeNode * t)
{ if (t==NULL) return;
  else return;
}

static ScopeList currentScope;

static void semanticError(TreeNode* t, const char* message, ...)
{
  va_list ap;
  va_start(ap, message);

  fprintf(listing, "Semantic Error: ");
  vfprintf(listing, message, ap);
  fprintf(listing, " at line %d\n", t->lineno);

  va_end(ap);

  Error = TRUE;
}

/* Procedure insertNode inserts 
 * identifiers stored in t into 
 * the symbol table 
 */
static int func_decl_flag = 0;

static void insertNode( TreeNode * t)
{
  BucketList l;
  switch (t->nodekind)
  { case StmtK:
      switch (t->kind.stmt)
      { 
        case VoidParamK:
          break;

        case ParamK:
        case VarDeclK:
          if ((l = st_lookup(currentScope, t->attr.name)) != NULL && l->scope == currentScope)
          {
            semanticError(t, "redefined variable '%s'", t->attr.name);
            break;
          }

          st_insert(currentScope, t->attr.name, t->type, t->lineno, t->lineno);
          break;

        case FuncDeclK:
          if ((l = st_lookup(currentScope, t->attr.name)) != NULL && l->scope == currentScope)
          {
            semanticError(t, "redefined function '%s'", t->attr.name);
            break;
          }

          {
            TreeNode* param = t->child[0];
            l = st_insert(currentScope, t->attr.name, Function, t->lineno, t->lineno);

            l->func.type = t->type;
            if (param->type != Void)
            {
              for (l->func.params = 0; l->func.params < MAX_FUNC_PARAMS; l->func.params++)
              {
                if (param == NULL)
                  break;

                l->func.param[l->func.params].type = param->type;
                l->func.param[l->func.params].name = copyString(param->attr.name);
                param = param->sibling;
              }
            }

            ScopeList newScope = buildScope(t->attr.name, currentScope);
            addChildScope(currentScope, newScope);
            currentScope = newScope;

            func_decl_flag = 1;
          }

          break;

        case CompoundK:
          if (func_decl_flag == 1)
          {
            func_decl_flag = 0;
          }
          else
          {
            char buf[256];
            sprintf(buf, "%s-%d", currentScope->name, t->lineno);

            ScopeList newScope = buildScope(buf, currentScope);
            addChildScope(currentScope, newScope);
            currentScope = newScope;
          }
          break;

        case ReturnK:
        case WhileK:
        case IfK:
        case IfElseK:
        default:
          break;
      }
      break;
    case ExpK:
      switch (t->kind.exp)
      {
        case OpK:
        case ConstK:
        case AssignK:
          break;

        case VarAccessK:
        case CallK:
          {
            if ((l = st_lookup(currentScope, t->attr.name)) == NULL)
            {
              semanticError(t, "undefined identifier '%s'", t->attr.name);
              break;
            }

            st_insert(l->scope, t->attr.name, l->type, t->lineno, t->lineno);
          }
          break;
      }
      break;
  }
}

static void afterInsertNode( TreeNode* t )
{
  if (t->nodekind == StmtK && t->kind.stmt == CompoundK)
  {
    currentScope = currentScope->parent;
  }
}

/* Function buildSymtab constructs the symbol 
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode * syntaxTree)
{ init_symtab();
  extern ScopeList globalScope;
  currentScope = globalScope;
  traverse(syntaxTree,insertNode,afterInsertNode);
  if (TraceAnalyze)
  { fprintf(listing,"\n< Symbol Table >\n");
    printSymTab(listing);
    fprintf(listing,"\n< Function Table >\n");
    printFuncTab(listing);
    fprintf(listing,"\n< Function and Global Variables >\n");
    printFuncAndGlobalTab(listing);
    fprintf(listing,"\n< Local Variables >\n");
    printLocalVarTab(listing);
  }
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void beforeCheckNode(TreeNode* t)
{
  if (t->nodekind == StmtK)
  {
    if (t->kind.stmt == FuncDeclK)
    {
      currentScope = findScope(t->attr.name, currentScope);
      func_decl_flag = 1;
    }
    else if (t->kind.stmt == CompoundK)
    {
      if (func_decl_flag == 1)
      {
        func_decl_flag = 0;
      }
      else
      {
        char buf[256];
        sprintf(buf, "%s-%d", currentScope->name, t->lineno);

        currentScope = findScope(buf, currentScope);
      }
    }
  }
}

static void checkNode(TreeNode * t)
{
  extern ScopeList globalScope;

  switch (t->nodekind)
  {
    case StmtK:
      switch (t->kind.stmt)
      {
        case VoidParamK:
          break;

        case ParamK:
          if (t->type == Void || t->type == VoidArr)
          {
            semanticError(t, "invalid type '%s' for parameter '%s'", typestr(t->type), t->attr.name);
            break;
          }
          break;

        case VarDeclK:
          if (t->type == Void || t->type == VoidArr)
          {
            semanticError(t, "invalid type '%s' for variable '%s'", typestr(t->type), t->attr.name);
            break;
          }
          break;

        case FuncDeclK:
          break;

        case CompoundK:
          currentScope = currentScope->parent;
          break;

        case IfK:
        case IfElseK:
        case WhileK:
          if (t->child[0]->type != Integer)
          {
            semanticError(t->child[0], "invalid type '%s' for condition", typestr(t->child[0]->type));
            break;
          }
          break;

        case ReturnK:
        {
          ScopeList sc = currentScope;
          while (sc->parent != globalScope)
          {
            sc = sc->parent;
          }
          BucketList l = st_lookup(globalScope, sc->name);

          if (t->child[0] != NULL)
          {
            if (l->func.type == Void)
            {
              semanticError(t->child[0], "return with a value, in function returing void");
              break;
            }
            else if (t->child[0]->type != l->func.type)
            {
              semanticError(t->child[0], "return type mismatch, expected '%s'", typestr(l->type));
              break;
            }
          }
          else if (l->func.type != Void) // t->child[0] == NULL
          {
            semanticError(t, "return with no value, in function returning non-void");
            break;
          }
        }
      }
      break;
    case ExpK:
      switch (t->kind.exp)
      {
        case OpK:
          if (t->child[0]->type == ErrorExp || t->child[1]->type == ErrorExp)
          {
            t->type = ErrorExp;
            break;
          }

          if (t->child[0]->type != Integer ||
              t->child[1]->type != Integer)
          {
            semanticError(t, "not allowed operation between '%s' and '%s'",
              typestr(t->child[0]->type), typestr(t->child[1]->type));
            break;
          }

          t->type = Integer;
          break;

        case ConstK:
          t->type = Integer;
          break;

        case AssignK:
          if (t->child[0]->nodekind != ExpK)
          {
            semanticError(t, "left operand of assignment must be expression");
            break;
          }

          if (t->child[0]->type == ErrorExp || t->child[1]->type == ErrorExp)
          {
            t->type = ErrorExp;
            break;
          }

          if (t->child[0]->kind.exp != VarAccessK ||
              t->child[0]->type == Function ||
              t->child[0]->type == IntegerArr ||
              t->child[0]->type == VoidArr)
          {
            semanticError(t, "lvalue required as left operand of assignment");
            break;
          }

          if (t->child[0]->type != t->child[1]->type)
          {
            semanticError(t, "type mismatch between left and right operand of assignment");
            break;
          }

          t->type = t->child[0]->type;

          break;

        case VarAccessK:
        {
          BucketList l = st_lookup(currentScope, t->attr.name);
          if (l == NULL)
          {
            t->type = ErrorExp;
            break;
          }
          
          t->type = l->type;

          if (t->child[0] != NULL)
          {
            if (t->child[0]->type == ErrorExp)
            {
              t->type = ErrorExp;
              break;
            }

            if (l->type == IntegerArr || l->type == VoidArr)
            {
              if (t->child[0]->type != Integer)
              {
                semanticError(t, "array index must be integer");
                break;
              }
              t->type -= 2;
            }
            else
            {
              semanticError(t, "array index is not allowed for non-array variable");
              break;
            }
          }

          break;
        }

        case CallK:
        {
          BucketList l = st_lookup(currentScope, t->attr.name);
          if (l == NULL)
          {
            t->type = ErrorExp;
            break;
          }

          int params = 0;
          TreeNode* param = t->child[0];
          while (param != NULL)
          {
            params++;
            param = param->sibling;
          }

          if (params < l->func.params)
          {
            semanticError(t, "too few arguments for function '%s'", t->attr.name);
            break;
          }
          else if (params > l->func.params)
          {
            semanticError(t, "too many arguments to function '%s'", t->attr.name);
            break;
          }

          param = t->child[0];
          for (int p = 0; p < params; ++p)
          {
            if (param->type != l->func.param[p].type)
            {
              semanticError(t, "type mismatch between parameter '%s' and argument '%s'",
                l->func.param[p].name, param->attr.name);
              break;
            }

            param = param->sibling;
          }

          t->type = l->func.type;

          break;
        }
      }
      break;
  }
}

/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode * syntaxTree)
{ traverse(syntaxTree,beforeCheckNode,checkNode);
}
