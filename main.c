#include "common.h"
#include "parser.h"
#include "cnf.h"
#include "solver.h"
#include <stdio.h>

/**
 * main.c - Program entry point
 */

/* Global variable for tracking syntax errors */
int syntax_error_occurred = 0;

/* Error reporting function */
void yyerror(const char *msg) {
  fprintf(stderr, "error: %s\n", msg);
  syntax_error_occurred = 1;
}

/* Process the parsed input */
void process_input(CNF *cnf) {
  if (!cnf || cnf->count < 0) {
    printf("NO-SOLUTION\n");
    if (cnf) free_cnf(cnf);
    return;
  }

  if (cnf->count == 0) {
    printf("SATISFACIBLE\n");
    free_cnf(cnf);
    return;
  }

  for (int i = 0; i < cnf->count; i++) {
    if (cnf->clauses[i].count < 0) {
      printf("NO-SOLUTION\n");
      free_cnf(cnf);
      return;
    }
  }

  Assignment *assn = create_assignment();
  solve(cnf, assn);

  free_assignment(assn);
  free_cnf(cnf);
}

/* Main function */
int main(int argc, char **argv) {
  syntax_error_occurred = 0;

  int result = yyparse();

  if (syntax_error_occurred) {
    printf("\nNO-SOLUTION\n");
  }

  yylex_destroy();
  return result;
}
