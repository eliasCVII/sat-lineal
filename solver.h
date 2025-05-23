#ifndef SOLVER_H
#define SOLVER_H

/**
 * solver.h - Solver interface (DPLL algorithm, assignments)
 */

#include "cnf.h"

/* Assignment structure (variable assignments) */
typedef struct Assignment {
  char** variables;   /* Array of variable names */
  int* values;        /* Array of values (0 or 1) */
  int size;           /* Number of variables */
} Assignment;

/* Assignment management functions */
Assignment *create_assignment();
void assign_variable(Assignment *assn, char *var, int value);
int get_variable_value(Assignment *assn, char *var);
void free_assignment(Assignment *assn);
Assignment *copy_assignment(Assignment *orig);

/* Propagation functions */
int find_forced_assignments(CNF *cnf, Assignment *assn, Assignment *forced);
char *pick_unassigned(CNF *cnf, Assignment *assn);

/* DPLL algorithm */
int dpll(CNF *cnf, Assignment *assn);

/* Main solver function */
void solve(CNF *cnf, Assignment *assn);

#endif /* SOLVER_H */
