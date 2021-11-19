/****************************************************/
/* File: cminus.y                                   */
/* The C-MINUS Yacc/Bison specification file        */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/
%{
#define YYPARSER /* distinguishes Yacc output from other code files */

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

#define YYSTYPE TreeNode *
static char * savedName; /* for use in assignments */
static int savedLineNo;  /* ditto */
static int savedNumber;
static TreeNode * savedTree; /* stores syntax tree for later return */
static int yylex(void); // added 11/2/11 to ensure no conflict with lex

%}

%token IF ELSE WHILE RETURN INT VOID
%token ID NUM 
%token ASSIGN EQ NE LT LE GT GE PLUS MINUS TIMES OVER LPAREN RPAREN LBRACE RBRACE LCURLY RCURLY SEMI COMMA
%token ERROR 

%% /* Grammar for C-MINUS */

program     : declaration_list
                 { savedTree = $1;} 
            ;

declaration_list : declaration_list declaration
                 { YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                         t = t->sibling;
                     t->sibling = $2;
                     $$ = $1; }
                     else $$ = $2;
                 }
            | declaration
                 { $$ = $1; }
            ;

declaration : var_declaration
                 { $$ = $1; }
            | fun_declaration
                 { $$ = $1; }
            ;

var_declaration : type_specifier ID
                  {
                    savedName = copyString(tokenString[tokenBit]);
                    savedLineNo = lineno;
                  }
                  SEMI
                  {
                    $$ = $1;
                    $$->kind.stmt = VarDeclK;
                    $$->attr.name = savedName;
                    $$->lineno = savedLineNo;
                  }
            | type_specifier ID
                  {
                    savedName = copyString(tokenString[tokenBit]);
                    savedLineNo = lineno;
                  }
                  LBRACE
                  NUM
                  {
                    savedNumber = atoi(tokenString[!tokenBit]);
                  }
                  RBRACE SEMI
                  {
                    $$ = $1;
                    $$->kind.stmt = VarDeclK;
                    $$->type += 2;
                    $$->attr.name = savedName;
                    $$->lineno = savedLineNo;

                    $$->child[0] = newExpNode(ConstK);
                    $$->child[0]->attr.val = savedNumber;
                  }
            ;

type_specifier : INT
                { $$ = newStmtNode(ParamK);
                  $$->type = Integer;
                }
             | VOID
                { $$ = newStmtNode(ParamK);
                  $$->type = Void;
                }
            ;

fun_declaration : type_specifier ID
                  {
                    savedName = copyString(tokenString[tokenBit]);
                    savedLineNo = lineno;
                  }
                  LPAREN params RPAREN compound_stmt
                  {
                    $$ = $1;
                    $$->kind.stmt = FuncDeclK;
                    $$->attr.name = savedName;
                    $$->lineno = savedLineNo;

                    $$->child[0] = $5;
                  }
            ;

params : param_list
        { $$ = $1; }
      | VOID
        { $$ = newStmtNode(VoidParamK); }
      ;

param_list : param_list COMMA param
        {
          YYSTYPE t = $1;
          if (t != NULL)
          {
            while (t->sibling != NULL)
              t = t->sibling;
            t->sibling = $3;
            $$ = $1; }
            else $$ = $3;
        }
      | param { $$ = $1; }
    ;

param : type_specifier ID
        {
          $$ = $1;
          $$->attr.name = copyString(tokenString[tokenBit]);
        }
      | type_specifier ID
        {
          savedName = copyString(tokenString[tokenBit]);
        }
        LBRACE RBRACE
        {
          $$ = $1;
          $$->attr.name = savedName;
          $$->type += 2;
        }
    ;

compound_stmt : %empty;

%%

int yyerror(char * message)
{ fprintf(listing,"Syntax error at line %d: %s\n",lineno,message);
  fprintf(listing,"Current token: ");
  printToken(yychar,tokenString[!tokenBit]);
  Error = TRUE;
  return 0;
}

/* yylex calls getToken to make Yacc/Bison output
 * compatible with ealier versions of the TINY scanner
 */
static int yylex(void)
{ return getToken(); }

TreeNode * parse(void)
{ yyparse();
  return savedTree;
}

