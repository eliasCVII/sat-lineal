#include "cnf.h"
#include "solver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * cnf.c - Implementation of CNF operations and transformations
 */

/* Transform an AST node to a literal */
Literal *ast_to_literal(struct ast *node) {
  if (!node)
    return NULL;

  Literal *lit = malloc(sizeof(Literal));
  lit->negated = 0;

  if (node->type == NODE_VAR) {
    lit->var = strdup(node->data.var_name);
  } else if (node->type == NODE_NOT) {
    lit->negated = 1;
    struct ast *child = node->data.child;
    if (child->type == NODE_VAR) {
      lit->var = strdup(child->data.var_name);
    } else {
      free(lit);
      return NULL;
    }
  } else {
    free(lit);
    return NULL;
  }
  return lit;
}

/* Flatten an OR expression into a clause */
void flatten_or(struct ast *node, Clause *clause) {
  if (!node || node->type != NODE_OR)
    return;

  if (node->data.binop.left->type == NODE_OR) {
    flatten_or(node->data.binop.left, clause);
  } else {
    Literal *lit = ast_to_literal(node->data.binop.left);
    if (lit) {
      clause->literals =
          realloc(clause->literals, (clause->count + 1) * sizeof(Literal));
      clause->literals[clause->count].var = strdup(lit->var);
      clause->literals[clause->count].negated = lit->negated;
      clause->count++;
      free_literal(lit);
    }
  }

  if (node->data.binop.right->type == NODE_OR) {
    flatten_or(node->data.binop.right, clause);
  } else {
    Literal *lit = ast_to_literal(node->data.binop.right);
    if (lit) {
      clause->literals =
          realloc(clause->literals, (clause->count + 1) * sizeof(Literal));
      clause->literals[clause->count].var = strdup(lit->var);
      clause->literals[clause->count].negated = lit->negated;
      clause->count++;
      free_literal(lit);
    }
  }
}

/* Convert an AST in CNF form to a CNF structure */
CNF *ast_to_cnf(struct ast *node) {
  if (!node)
    return NULL;

  CNF *cnf = malloc(sizeof(CNF));
  cnf->clauses = NULL;
  cnf->count = 0;

  if (node->type == NODE_AND) {
    CNF *left_cnf = ast_to_cnf(node->data.binop.left);
    CNF *right_cnf = ast_to_cnf(node->data.binop.right);

    cnf->count = left_cnf->count + right_cnf->count;
    cnf->clauses = malloc(cnf->count * sizeof(Clause));

    for (int i = 0; i < left_cnf->count; i++) {
      Clause *src = &left_cnf->clauses[i];
      Clause *dest = &cnf->clauses[i];
      dest->count = src->count;
      dest->literals = malloc(dest->count * sizeof(Literal));
      for (int j = 0; j < src->count; j++) {
        dest->literals[j].var = strdup(src->literals[j].var);
        dest->literals[j].negated = src->literals[j].negated;
      }
    }

    for (int i = 0; i < right_cnf->count; i++) {
      Clause *src = &right_cnf->clauses[i];
      Clause *dest = &cnf->clauses[left_cnf->count + i];
      dest->count = src->count;
      dest->literals = malloc(dest->count * sizeof(Literal));
      for (int j = 0; j < src->count; j++) {
        dest->literals[j].var = strdup(src->literals[j].var);
        dest->literals[j].negated = src->literals[j].negated;
      }
    }

    free_cnf(left_cnf);
    free_cnf(right_cnf);
  } else if (node->type == NODE_OR) {
    Clause *clause = malloc(sizeof(Clause));
    clause->literals = NULL;
    clause->count = 0;
    flatten_or(node, clause);

    cnf->count = 1;
    cnf->clauses = malloc(sizeof(Clause));
    cnf->clauses[0] = *clause;
    free(clause);
  } else {
    Literal *lit = ast_to_literal(node);
    if (lit) {
      Clause *clause = malloc(sizeof(Clause));
      clause->count = 1;
      clause->literals = malloc(sizeof(Literal));
      clause->literals[0].var = strdup(lit->var);
      clause->literals[0].negated = lit->negated;

      cnf->count = 1;
      cnf->clauses = malloc(sizeof(Clause));
      cnf->clauses[0] = *clause;

      free_literal(lit);
      free(clause);
    }
  }

  return cnf;
}

/* Evaluate a clause with a given assignment */
int evaluate_clause(Clause *clause, Assignment *assn) {
  int has_unassigned = 0;
  for (int i = 0; i < clause->count; i++) {
    Literal lit = clause->literals[i];
    int value = get_variable_value(assn, lit.var);

    if (value == -1) {
      has_unassigned = 1;
      continue;
    }

    if (value != lit.negated) {
      return 1;  /* Clause is satisfied */
    }
  }
  return has_unassigned ? -1 : 0;  /* -1: undetermined, 0: unsatisfied */
}

/* Evaluate a CNF formula with a given assignment */
int evaluate_cnf(CNF *cnf, Assignment *assn) {
  for (int i = 0; i < cnf->count; i++) {
    int result = evaluate_clause(&cnf->clauses[i], assn);
    if (result == 0)
      return 0;  /* Formula is unsatisfied */
    if (result == -1)
      return -1; /* Formula is undetermined */
  }
  return 1;  /* Formula is satisfied */
}

/* Print a CNF formula */
void print_cnf(CNF *cnf) {
  printf("CNF has %d clauses:\n", cnf->count);
  for (int i = 0; i < cnf->count; i++) {
    printf("Clause %d: ", i + 1);
    for (int j = 0; j < cnf->clauses[i].count; j++) {
      Literal lit = cnf->clauses[i].literals[j];
      printf("%s%s ", lit.negated ? "¬" : "", lit.var);
    }
    printf("\n");
  }
}

/* Free a literal */
void free_literal(Literal *lit) {
  if (lit) {
    free(lit->var);
    free(lit);
  }
}

/* Free a clause */
void free_clause(Clause *clause) {
  if (clause) {
    for (int i = 0; i < clause->count; i++) {
      free(clause->literals[i].var);
    }
    free(clause->literals);
  }
}

/* Free a CNF formula */
void free_cnf(CNF *cnf) {
  if (cnf) {
    for (int i = 0; i < cnf->count; i++) {
      for (int j = 0; j < cnf->clauses[i].count; j++) {
        free(cnf->clauses[i].literals[j].var);
      }
      free(cnf->clauses[i].literals);
    }
    free(cnf->clauses);
    free(cnf);
  }
}
