%{
#include <stdio.h>
#include <stdlib.h>
#include "lexerast.h"
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
    printf("\n> Valid Expression\n\n");
    struct ast* nnf = to_nnf($2);

    printf("> AST: \n");
    print_ast($2);
    printf("\n");

    printf("> Translated AST: \n");
    print_ast(nnf);

    printf("\n\n> Translated AST (LaTeX): \n$$ ");
    print_ast_latex(nnf);
    printf(" $$\n");

    free_ast(nnf);
    free_ast($2);
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
