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
static char * savedName[1024]; /* for use in assignments */
static int saveNamePos = 0;
static int savedLineNo[1024];  /* ditto */
static int saveLineNoPos = 0;
static int savedNumber[1024];
static int saveNumberPos = 0;
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
                    savedName[saveNamePos++] = copyString(tokenString[tokenBit]);
                    savedLineNo[saveLineNoPos++] = lineno;
                  }
                  SEMI
                  {
                    $$ = $1;
                    $$->kind.stmt = VarDeclK;
                    $$->attr.name = savedName[--saveNamePos];
                    $$->lineno = savedLineNo[--saveLineNoPos];
                  }
            | type_specifier ID
                  {
                    savedName[saveNamePos++] = copyString(tokenString[tokenBit]);
                    savedLineNo[saveLineNoPos++] = lineno;
                  }
                  LBRACE
                  NUM
                  {
                    savedNumber[saveNumberPos++] = atoi(tokenString[!tokenBit]);
                  }
                  RBRACE SEMI
                  {
                    $$ = $1;
                    $$->kind.stmt = VarDeclK;
                    $$->type += 2;
                    $$->attr.name = savedName[--saveNamePos];
                    $$->lineno = savedLineNo[--saveLineNoPos];

                    $$->child[0] = newExpNode(ConstK);
                    $$->child[0]->attr.val = savedNumber[--saveNumberPos];
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
                    savedName[saveNamePos++] = copyString(tokenString[tokenBit]);
                    savedLineNo[saveLineNoPos++] = lineno;
                  }
                  LPAREN params RPAREN compound_stmt
                  {
                    $$ = $1;
                    $$->kind.stmt = FuncDeclK;
                    $$->attr.name = savedName[--saveNamePos];
                    $$->lineno = savedLineNo[--saveLineNoPos];

                    $$->child[0] = $5;
                    $$->child[1] = $7;
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
          savedName[saveNamePos++] = copyString(tokenString[tokenBit]);
        }
        LBRACE RBRACE
        {
          $$ = $1;
          $$->attr.name = savedName[--saveNamePos];
          $$->type += 2;
        }
    ;

compound_stmt : LCURLY local_declarations statement_list RCURLY
        {
          $$ = newStmtNode(CompoundK);
          $$->child[0] = $2;
          $$->child[1] = $3;
        }
    ;

local_declarations : local_declarations var_declaration
        {
          YYSTYPE t = $1;
          if (t != NULL)
          {
            while (t->sibling != NULL)
              t = t->sibling;
            t->sibling = $2;
            $$ = $1; }
            else $$ = $2;
        }
        | /* empty */
        { $$ = NULL; }
    ;

statement_list : statement_list statement
        {
          YYSTYPE t = $1;
          if (t != NULL)
          {
            while (t->sibling != NULL)
              t = t->sibling;
            t->sibling = $2;
            $$ = $1; }
            else $$ = $2;
        }
        | /* empty */
        { $$ = NULL; }
    ;

statement : expression_stmt
        { $$ = $1; }
      | compound_stmt
        { $$ = $1; }
      | selection_stmt
        { $$ = $1; }
      | iteration_stmt
        { $$ = $1; }
      | return_stmt
        { $$ = $1; }
    ;

expression_stmt : expression SEMI
        { $$ = $1; }
      | SEMI
        { $$ = NULL; }
    ;

selection_stmt : IF LPAREN expression RPAREN statement
        {
          $$ = newStmtNode(IfK);
          $$->child[0] = $3;
          $$->child[1] = $5;
        }
      | IF LPAREN expression RPAREN statement ELSE statement
        {
          $$ = newStmtNode(IfElseK);
          $$->child[0] = $3;
          $$->child[1] = $5;
          $$->child[2] = $7;
        }
    ;

iteration_stmt : WHILE LPAREN expression RPAREN statement
        {
          $$ = newStmtNode(WhileK);
          $$->child[0] = $3;
          $$->child[1] = $5;
        }
    ;

return_stmt : RETURN SEMI
        { $$ = newStmtNode(ReturnK); }
      | RETURN expression SEMI
        {
          $$ = newStmtNode(ReturnK);
          $$->child[0] = $2;
        }  
    ;

expression : var ASSIGN expression
        {
          $$ = newExpNode(AssignK);
          $$->child[0] = $1;
          $$->child[1] = $3;
        }
      | simple_expression
        { $$ = $1; }
    ;

var : ID
        {
          $$ = newExpNode(VarAccessK);
          $$->attr.name = copyString(tokenString[tokenBit]);
        }
      | ID
        {
          savedName[saveNamePos++] = copyString(tokenString[tokenBit]);
        }
        LBRACE expression RBRACE
        {
          $$ = newExpNode(VarAccessK);
          $$->attr.name = savedName[--saveNamePos];
          $$->child[0] = $4;
        }
    ;

simple_expression : additive_expression relop additive_expression
        {
          $$ = $2;
          $$->child[0] = $1;
          $$->child[1] = $3;
        }
      | additive_expression
        { $$ = $1; }
    ;

relop : LE
        {
          $$ = newExpNode(OpK);
          $$->attr.op = LE;
        }
      | LT
        {
          $$ = newExpNode(OpK);
          $$->attr.op = LT;
        }
      | GT
        {
          $$ = newExpNode(OpK);
          $$->attr.op = GT;
        }
      | GE
        {
          $$ = newExpNode(OpK);
          $$->attr.op = GE;
        }
      | EQ
        {
          $$ = newExpNode(OpK);
          $$->attr.op = EQ;
        }
      | NE
        {
          $$ = newExpNode(OpK);
          $$->attr.op = NE;
        }
    ;

additive_expression : additive_expression addop term
        {
          $$ = $2;
          $$->child[0] = $1;
          $$->child[1] = $3;
        }
      | term
        { $$ = $1; }
    ;

addop : PLUS
        {
          $$ = newExpNode(OpK);
          $$->attr.op = PLUS;
        }
      | MINUS
        {
          $$ = newExpNode(OpK);
          $$->attr.op = MINUS;
        }
    ;

term : term mulop factor
        {
          $$ = $2;
          $$->child[0] = $1;
          $$->child[1] = $3;
        }
      | factor
        { $$ = $1; }
    ;

mulop : TIMES
        {
          $$ = newExpNode(OpK);
          $$->attr.op = TIMES;
        }
      | OVER
        {
          $$ = newExpNode(OpK);
          $$->attr.op = OVER;
        }
    ;

factor : LPAREN expression RPAREN
        {
          $$ = $2;
        }
      | var
        { $$ = $1; }
      | call
        { $$ = $1; }
      | NUM
        {
          $$ = newExpNode(ConstK);
          $$->attr.val = atoi(tokenString[!tokenBit]);
        }
    ;

call : ID
        {
          savedName[saveNamePos++] = copyString(tokenString[tokenBit]);
        }
        LPAREN args RPAREN
        {
          $$ = newExpNode(CallK);
          $$->attr.name = savedName[--saveNamePos];
          $$->child[0] = $4;
        }
    ;

args : arg_list
        { $$ = $1; }
      | /* empty */
        { $$ = NULL; }
    ;

arg_list : arg_list COMMA expression
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
      | expression
        { $$ = $1; }
    ;

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

