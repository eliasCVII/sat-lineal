%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  /* For strdup */
#include "sat-header.h"
%}

%union{
  char* atomo;
  struct ast *ast_node;
}

%token DELIM IMPLIES OR AND NOT LPAREN RPAREN
%token <atomo> VAR

%type <ast_node> exp implies_exp or_exp and_exp not_exp

%destructor { free($$); } <atomo>;
%destructor { free_ast($$); } <ast_node>;

%%
input:
  DELIM exp DELIM {
    // We'll skip printing the original AST to avoid memory issues
    // The original AST is already printed by the lexer/parser during parsing

    // Transform the AST to CNF
    struct ast* cnf = transform($2);
    free_ast($2); // Free the original AST
    CNF* flat_cnf = ast_to_cnf(cnf);

    // Skip printing the original AST

    // printf("\n\n");
    // printf("> Translated AST: \n");
    // print_ast(cnf);

    // printf("\n\n> (LaTeX): \n$$ ");
    // print_ast_latex(cnf);
    // printf(" $$\n");

    // printf("\n\n");
    // print_cnf(flat_cnf);

    printf("\n");
    process_input(flat_cnf);

    free_ast(cnf);
  }
  | DELIM /* empty */ DELIM {
    CNF* empty = malloc(sizeof(CNF));
    empty->clauses=NULL;
    empty->count=0;
    process_input(empty);
  }
  ;

exp:
  implies_exp { $$ = $1; }
  ;

implies_exp:
  or_exp { $$ = $1; }
  | or_exp IMPLIES implies_exp { $$ = make_binary_node(NODE_IMPLIES, $1, $3); }
  ;

or_exp:
  and_exp { $$ = $1; }
  | or_exp OR and_exp { $$ = make_binary_node(NODE_OR, $1, $3); }
  ;

and_exp:
  not_exp { $$ = $1; }
  | and_exp AND not_exp { $$ = make_binary_node(NODE_AND, $1, $3); }
  ;

not_exp:
  VAR { $$ = make_var_node($1);}
  | NOT not_exp { $$ = make_unary_node(NODE_NOT, $2);}
  | LPAREN exp RPAREN {$$ = $2;}
  ;
%%
