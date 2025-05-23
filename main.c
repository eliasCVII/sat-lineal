#include "common.h"
#include "parser.h"
#include "linear_solver.h"
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * main.c - Program entry point for the SAT solver using linear-time algorithm
 */

/* Global variable for tracking syntax errors */
int syntax_error_occurred = 0;

/* Error reporting function */
void yyerror(const char *msg) {
  fprintf(stderr, "error: %s\n", msg);
  syntax_error_occurred = 1;
}

/* Process the parsed input */
void process_input(ast *formula) {
  if (!formula) {
    printf("NO-SOLUTION\n");
    return;
  }

  /* Create assignment structure */
  LinearAssignment *assn = create_linear_assignment();

  /* Solve using linear solver */
  int result = linear_solve(formula, assn);

  /* Print result */
  printf(result ? "SATISFACIBLE\n" : "NO-SATISFACIBLE\n");

  /* Free resources */
  free_linear_assignment(assn);
  free_ast(formula);
}

/* Main function */
int main(int argc, char **argv) {
  syntax_error_occurred = 0;

  /* Parse input */
  ast *formula = parse_input_linear();

  // print_ast(formula);

  if (syntax_error_occurred) {
    printf("NO-SOLUTION\n");
    if (formula) free_ast(formula);
  } else {
    /* Process the parsed formula */
    process_input(formula);
  }

  yylex_destroy();
  return 0;
}
