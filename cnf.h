#ifndef CNF_H
#define CNF_H

/**
 * cnf.h - Conjunctive Normal Form data structures and interface
 */

#include "ast.h"

/* Maximum number of clauses and literals */
#define MAX_CLAUSES 100
#define MAX_LITERALS 100

/* Forward declaration for Assignment (defined in solver.h) */
typedef struct Assignment Assignment;

/* Literal structure (variable with negation flag) */
typedef struct Literal {
  char* var;     /* Variable name */
  int negated;   /* 1 if negated, 0 otherwise */
} Literal;

/* Clause structure (disjunction of literals) */
typedef struct Clause {
  Literal* literals;  /* Array of literals */
  int count;          /* Number of literals */
} Clause;

/* CNF structure (conjunction of clauses) */
typedef struct CNF {
  Clause* clauses;    /* Array of clauses */
  int count;          /* Number of clauses */
} CNF;

/* CNF transformation functions */
struct ast* to_cnf(struct ast* node);
Literal* ast_to_literal(struct ast* node);
void flatten_or(struct ast* node, Clause* clause);
CNF* ast_to_cnf(struct ast* node);

/* CNF evaluation functions */
int evaluate_clause(Clause *clause, Assignment *assn);
int evaluate_cnf(CNF *cnf, Assignment *assn);

/* CNF printing functions */
void print_cnf(CNF *cnf);

/* Memory management */
void free_literal(Literal* lit);
void free_clause(Clause* clause);
void free_cnf(CNF* cnf);

#endif /* CNF_H */
