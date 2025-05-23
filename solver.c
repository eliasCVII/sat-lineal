#include "solver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * solver.c - Implementation of the DPLL SAT solver
 */

/* Create a new empty assignment */
Assignment *create_assignment() {
  Assignment *assn = malloc(sizeof(Assignment));
  assn->variables = NULL;
  assn->values = NULL;
  assn->size = 0;
  return assn;
}

/* Assign a value to a variable */
void assign_variable(Assignment *assn, char *var, int value) {
  for (int i = 0; i < assn->size; i++) {
    if (strcmp(assn->variables[i], var) == 0) {
      assn->values[i] = value;
      return;
    }
  }

  assn->size++;
  assn->variables = realloc(assn->variables, assn->size * sizeof(char *));
  assn->values = realloc(assn->values, assn->size * sizeof(int));
  assn->variables[assn->size - 1] = strdup(var);
  assn->values[assn->size - 1] = value;
}

/* Get the value of a variable (-1 if unassigned) */
int get_variable_value(Assignment *assn, char *var) {
  for (int i = 0; i < assn->size; i++) {
    if (strcmp(assn->variables[i], var) == 0) {
      return assn->values[i];
    }
  }
  return -1;
}

/* Free an assignment */
void free_assignment(Assignment *assn) {
  if (assn) {
    for (int i = 0; i < assn->size; i++) {
      free(assn->variables[i]);
    }
    free(assn->variables);
    free(assn->values);
    free(assn);
  }
}

/* Create a copy of an assignment */
Assignment *copy_assignment(Assignment *orig) {
  Assignment *c = create_assignment();
  for (int i = 0; i < orig->size; i++)
    assign_variable(c, orig->variables[i], orig->values[i]);
  return c;
}

/* Find forced assignments (unit propagation) */
int find_forced_assignments(CNF *cnf, Assignment *assn, Assignment *forced) {
  for (int i = 0; i < cnf->count; i++) {
    Clause *clause = &cnf->clauses[i];
    int unassigned_count = 0;
    Literal *unit_literal = NULL;
    int clause_satisfied = 0;

    for (int j = 0; j < clause->count; j++) {
      Literal lit = clause->literals[j];
      int value = get_variable_value(assn, lit.var);

      if (value == -1) {
        unassigned_count++;
        unit_literal = &clause->literals[j];
      } else if (value != lit.negated) {
        clause_satisfied = 1;
        break;
      }
    }

    if (clause_satisfied)
      continue;

    if (unassigned_count == 0) {
      return 0;  /* Clause cannot be satisfied */
    } else if (unassigned_count == 1) {
      Literal lit = *unit_literal;
      int required_value = lit.negated ? 0 : 1;

      int current_value = get_variable_value(assn, lit.var);
      if (current_value != -1 && current_value != required_value) {
        return 0;  /* Contradiction */
      }

      int existing = get_variable_value(forced, lit.var);
      if (existing == -1) {
        assign_variable(forced, lit.var, required_value);
      } else if (existing != required_value) {
        return 0;  /* Contradiction in forced assignments */
      }
    }
  }
  return 1;  /* No contradictions found */
}

/* Pick an unassigned variable */
char *pick_unassigned(CNF *cnf, Assignment *assn) {
  for (int i = 0; i < cnf->count; i++) {
    for (int j = 0; j < cnf->clauses[i].count; j++) {
      Literal lit = cnf->clauses[i].literals[j];
      if (get_variable_value(assn, lit.var) == -1)
        return lit.var;
    }
  }
  return NULL;  /* All variables are assigned */
}

/* DPLL algorithm implementation */
int dpll(CNF *cnf, Assignment *assn) {
  if (cnf->count == 0)
    return 1;  /* Empty CNF is satisfiable */

  int ev = evaluate_cnf(cnf, assn);
  if (ev == 1)
    return 1;  /* Formula is satisfied */
  if (ev == 0)
    return 0;  /* Formula is unsatisfied */

  /* Unit propagation */
  Assignment *forced = create_assignment();
  if (!find_forced_assignments(cnf, assn, forced)) {
    free_assignment(forced);
    return 0;  /* Contradiction found */
  }
  for (int i = 0; i < forced->size; i++)
    assign_variable(assn, forced->variables[i], forced->values[i]);
  free_assignment(forced);

  /* Pick an unassigned variable */
  char *var = pick_unassigned(cnf, assn);
  if (!var)
    return 1;  /* All variables assigned and no contradictions */

  /* Try both possible values */
  for (int val = 1; val >= 0; val--) {
    Assignment *as2 = copy_assignment(assn);
    assign_variable(as2, var, val);
    if (dpll(cnf, as2)) {
      /* Copy the successful assignment back to the original */
      for (int i = 0; i < as2->size; i++) {
        assign_variable(assn, as2->variables[i], as2->values[i]);
      }
      free_assignment(as2);
      return 1;
    }
    free_assignment(as2);
  }

  return 0;  /* No solution found */
}

/* Main solver function */
void solve(CNF *cnf, Assignment *assn) {
  int res = dpll(cnf, assn);
  printf(res ? "SATISFACIBLE\n" : "NO-SATISFACIBLE\n");
}
