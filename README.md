# ELE4029
Compiler Construction @ Hanyang Univ.

## [Project1](https://github.com/frechele/ELE4029/tree/main/1_Scanner)
- C-minus scanner implementations.
- The scanner read an input source code string, tokenize it, and return recognized tokens.
- Two version exists.
  - Implementation method1: recognize tokens by DFA.
  - Implementation method2: specify lexical patterns by Regular Expression.

## [Project2](https://github.com/frechele/ELE4029/tree/main/2_Parser)
- C-minus parser implementation using Yacc (bison).
- The parser read an input source code string, tokenize & parse it with C-minus grammar, and return abstract syntax tree (AST).

## [Project3](https://github.com/frechele/ELE4029/tree/main/3_Semantic)
- C-minus semantic analyzer implementation.
- Find all semantic errors using symbol table & type checker.
- The semantic analyzer read an input source code string, and generate AST. After that, the semantic analyzer traverses the AST to find and print semantic errors and its line number.
