/****************************************************/
/* File: cminus.l                                   */
/* Lex specification for C-MINUS                    */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

%{
#include "globals.h"
#include "util.h"
#include "scan.h"
/* lexeme of identifier or reserved word */
char tokenString[2][MAXTOKENLEN+1];
int tokenBit = 0;
%}

digit       [0-9]
number      {digit}+
letter      [a-zA-Z]
identifier  {letter}({letter}|{digit})*
newline     \n
whitespace  [ \t]+

%%

"if"            {return IF;}
"else"          {return ELSE;}
"while"         {return WHILE;}
"return"        {return RETURN;}
"int"           {return INT;}
"void"          {return VOID;}
"=="            {return EQ;}
"!="            {return NE;}
"="             {return ASSIGN;}
"<"             {return LT;}
"<="            {return LE;}
">"             {return GT;}
">="            {return GE;}
"+"             {return PLUS;}
"-"             {return MINUS;}
"*"             {return TIMES;}
"/"             {return OVER;}
"("             {return LPAREN;}
")"             {return RPAREN;}
"["             {return LBRACE;}
"]"             {return RBRACE;}
"{"             {return LCURLY;}
"}"             {return RCURLY;}
";"             {return SEMI;}
","             {return COMMA;}
{number}        {return NUM;}
{identifier}    {return ID;}
{newline}       {lineno++;}
{whitespace}    {/* skip whitespace */}
"/*"            { char c, pre;
                  do
                  { pre = c;
                    c = input();
                    if (c == EOF || c == '\0') break;
                    if (c == '\n') lineno++;
                  } while (pre != '*' || c != '/');
                }
.               {return ERROR;}

%%

TokenType getToken(void)
{ static int firstTime = TRUE;
  TokenType currentToken;
  if (firstTime)
  { firstTime = FALSE;
    lineno++;
    yyin = source;
    yyout = listing;
  }
  currentToken = yylex();
  strncpy(tokenString[tokenBit],yytext,MAXTOKENLEN);
  if (TraceScan) {
    fprintf(listing,"\t%d: ",lineno);
    printToken(currentToken,tokenString[tokenBit]);
  }
  tokenBit = !tokenBit;
  return currentToken;
}

