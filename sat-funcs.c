#include "sat-header.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ast *make_var_node(char *name) {
  ast *node = malloc(sizeof(ast));
  node->type = NODE_VAR;
  node->data.var_name = name;
  return node;
}

struct ast *make_unary_node(NodeType type, ast *child) {
  ast *node = malloc(sizeof(ast));
  node->type = type;
  node->data.child = child;
  return node;
}

struct ast *make_binary_node(NodeType type, ast *l, ast *r) {
  ast *node = malloc(sizeof(ast));
  node->type = type;
  node->data.binop.left = l;
  node->data.binop.right = r;
  return node;
}

struct ast *transform(struct ast *node) {
  if (!node)
    return NULL;

  switch (node->type) {
  case NODE_VAR:
    return make_var_node(strdup(node->data.var_name));

  case NODE_NOT: {
    struct ast *child = transform(node->data.child);
    switch (child->type) {
    case NODE_NOT: // Double negation elimination
      return transform(child->data.child);
    case NODE_AND: // De Morgan: ¬(A ∧ B) → ¬A ∨ ¬B
      return make_binary_node(
          NODE_OR, transform(make_unary_node(NODE_NOT, child->data.binop.left)),
          transform(make_unary_node(NODE_NOT, child->data.binop.right)));
    case NODE_OR: // De Morgan: ¬(A ∨ B) → ¬A ∧ ¬B
      return make_binary_node(
          NODE_AND,
          transform(make_unary_node(NODE_NOT, child->data.binop.left)),
          transform(make_unary_node(NODE_NOT, child->data.binop.right)));
    default: // ¬A (literal)
      return make_unary_node(NODE_NOT, child);
    }
  }

  case NODE_IMPLIES: { // A → B ≡ ¬A ∨ B
    struct ast *left = transform(node->data.binop.left);
    struct ast *right = transform(node->data.binop.right);
    struct ast *new_left = make_unary_node(NODE_NOT, left);
    return distribute_OR(new_left, right);
  }

  case NODE_OR: {
    struct ast *left = transform(node->data.binop.left);
    struct ast *right = transform(node->data.binop.right);
    return distribute_OR(left, right);
  }

  case NODE_AND: {
    struct ast *left = transform(node->data.binop.left);
    struct ast *right = transform(node->data.binop.right);
    return make_binary_node(NODE_AND, left, right);
  }

  default:
    fprintf(stderr, "Unexpected node type in translate\n");
    return NULL;
  }
}

struct ast *distribute_OR(struct ast *left, struct ast *right) {
  if (left->type == NODE_AND) {
    return make_binary_node(NODE_AND,
                            distribute_OR(left->data.binop.left, right),
                            distribute_OR(left->data.binop.right, right));
  }
  if (right->type == NODE_AND) {
    return make_binary_node(NODE_AND,
                            distribute_OR(left, right->data.binop.left),
                            distribute_OR(left, right->data.binop.right));
  }
  return make_binary_node(NODE_OR, left, right);
}

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
      return NULL; // Invalid structure (e.g., ¬(A ∧ B))
    }
  } else {
    free(lit);
    return NULL; // Not a literal
  }
  return lit;
}

void flatten_or(struct ast *node, Clause *clause) {
  if (!node || node->type != NODE_OR)
    return;

  // Process left child
  if (node->data.binop.left->type == NODE_OR) {
    flatten_or(node->data.binop.left, clause);
  } else {
    Literal *lit = ast_to_literal(node->data.binop.left);
    if (lit) {
      clause->literals =
          realloc(clause->literals, (clause->count + 1) * sizeof(Literal));
      // Deep copy the literal
      clause->literals[clause->count].var = strdup(lit->var);
      clause->literals[clause->count].negated = lit->negated;
      clause->count++;
      free_literal(lit);
    }
  }

  // Process right child (similar to left)
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

    // Free the original CNFs properly
    free_cnf(left_cnf);
    free_cnf(right_cnf);
} else if (node->type == NODE_OR) {
    // Convert OR node to a single clause
    Clause *clause = malloc(sizeof(Clause));
    clause->literals = NULL;
    clause->count = 0;
    flatten_or(node, clause);

    cnf->count = 1;
    cnf->clauses = malloc(sizeof(Clause));
    cnf->clauses[0] = *clause;
    free(clause);
  } else {
    // Single literal (VAR or NOT)
    Literal *lit = ast_to_literal(node);
    if (lit) {
      Clause *clause = malloc(sizeof(Clause));
      clause->count = 1;
      clause->literals = malloc(sizeof(Literal));
      clause->literals[0].var = strdup(lit->var); // Deep copy
      clause->literals[0].negated = lit->negated;

      cnf->count = 1;
      cnf->clauses = malloc(sizeof(Clause));
      cnf->clauses[0] = *clause;

      free_literal(lit);
      free(clause); // Only free the container, not the literals
    }
  }

  return cnf;
}

// Assignment Management
Assignment *create_assignment() {
  Assignment *assn = malloc(sizeof(Assignment));
  assn->variables = NULL;
  assn->values = NULL;
  assn->size = 0;
  return assn;
}

void assign_variable(Assignment *assn, char *var, int value) {
  // Check for existing assignment
  for (int i = 0; i < assn->size; i++) {
    if (strcmp(assn->variables[i], var) == 0) {
      assn->values[i] = value;
      return;
    }
  }

  // Add new variable
  assn->size++;
  assn->variables = realloc(assn->variables, assn->size * sizeof(char *));
  assn->values = realloc(assn->values, assn->size * sizeof(int));
  assn->variables[assn->size - 1] = strdup(var);
  assn->values[assn->size - 1] = value;
}

int get_variable_value(Assignment *assn, char *var) {
  for (int i = 0; i < assn->size; i++) {
    if (strcmp(assn->variables[i], var) == 0) {
      return assn->values[i];
    }
  }
  return -1; // Not assigned
}

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

// CNF Evaluation
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
      return 1; // Clause satisfied
    }
  }
  return has_unassigned ? -1 : 0;
}

int evaluate_cnf(CNF *cnf, Assignment *assn) {
  for (int i = 0; i < cnf->count; i++) {
    int result = evaluate_clause(&cnf->clauses[i], assn);
    if (result == 0)
      return 0; // Unsatisfied clause
    if (result == -1)
      return -1; // Undecided
  }
  return 1; // All clauses satisfied
}

// Unit Propagation
int find_forced_assignments(CNF *cnf, Assignment *assn, Assignment *forced) {
  for (int i = 0; i < cnf->count; i++) {
    Clause *clause = &cnf->clauses[i];
    int unassigned_count = 0;
    Literal *unit_literal = NULL;
    int clause_satisfied = 0;

    // Check clause status
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
      return 0; // Conflict
    } else if (unassigned_count == 1) {
      // Found unit clause
      Literal lit = *unit_literal;
      int required_value = lit.negated ? 0 : 1;

      // Check for conflicts
      int current_value = get_variable_value(assn, lit.var);
      if (current_value != -1 && current_value != required_value) {
        return 0;
      }

      // Add to forced assignments
      int existing = get_variable_value(forced, lit.var);
      if (existing == -1) {
        assign_variable(forced, lit.var, required_value);
      } else if (existing != required_value) {
        return 0;
      }
    }
  }
  return 1;
}

// DPLL recursion
// Make a deep copy of the assignment
Assignment *copy_assignment(Assignment *orig) {
  Assignment *c = create_assignment();
  for (int i = 0; i < orig->size; i++)
    assign_variable(c, orig->variables[i], orig->values[i]);
  return c;
}

// Pick first unassigned variable found in CNF
char *pick_unassigned(CNF *cnf, Assignment *assn) {
  for (int i = 0; i < cnf->count; i++) {
    for (int j = 0; j < cnf->clauses[i].count; j++) {
      Literal lit = cnf->clauses[i].literals[j];
      if (get_variable_value(assn, lit.var) == -1)
        return lit.var;
    }
  }
  return NULL;
}

int dpll(CNF *cnf, Assignment *assn) {
  if (cnf->count == 0)
    return 1;

  int ev = evaluate_cnf(cnf, assn);
  if (ev == 1)
    return 1;
  if (ev == 0)
    return 0;

  // unit-propagation
  Assignment *forced = create_assignment();
  if (!find_forced_assignments(cnf, assn, forced)) {
    free_assignment(forced);
    return 0;
  }
  for (int i = 0; i < forced->size; i++)
    assign_variable(assn, forced->variables[i], forced->values[i]);
  free_assignment(forced);

  // choose variable
  char *var = pick_unassigned(cnf, assn);
  if (!var)
    return 1; // no var left ⇒ all satisfied

  // branch on var = true / false
  for (int val = 1; val >= 0; val--) {
    Assignment *as2 = copy_assignment(assn);
    assign_variable(as2, var, val);
    if (dpll(cnf, as2)) {
      free_assignment(as2);
      return 1;
    }
    free_assignment(as2);
  }

  return 0;
}

// Solve wrapper: parse AST → CNF → DPLL → print
void solve(CNF *cnf, Assignment *assn) {
  int res = dpll(cnf, assn);
  printf(res ? "SAT\n" : "UNSAT\n");
}

// Memory Management
void free_ast(ast *node) {
  if (!node)
    return;
  switch (node->type) {
  case NODE_VAR:
    free(node->data.var_name);
    break;
  case NODE_NOT:
    free_ast(node->data.child);
    break;
  case NODE_AND:
  case NODE_OR:
  case NODE_IMPLIES:
    free_ast(node->data.binop.left);
    free_ast(node->data.binop.right);
    break;
  case NODE_PAREN:
    free_ast(node->data.child);
    break;
  }
  free(node);
}

void free_literal(Literal *lit) {
  if (lit)
    free(lit->var);
  free(lit);
}

void free_clause(Clause *clause) {
  if (clause) {
    for (int i = 0; i < clause->count; i++) {
      free(clause->literals[i].var); // Free each literal's string
    }
    free(clause->literals); // Free the literals array
  }
}

void free_cnf(CNF *cnf) {
  if (cnf) {
    for (int i = 0; i < cnf->count; i++) {
      // Free each literal's var string
      for (int j = 0; j < cnf->clauses[i].count; j++) {
        free(cnf->clauses[i].literals[j].var);
      }
      free(cnf->clauses[i].literals);
    }
    free(cnf->clauses);
    free(cnf);
  }
}

// Printing
void print_ast(ast *node) {
  if (!node)
    return;
  switch (node->type) {
  case NODE_VAR:
    printf("%s", node->data.var_name);
    break;
  case NODE_NOT:
    printf("(NOT ");
    print_ast(node->data.child);
    printf(")");
    break;
  case NODE_AND:
    printf("(");
    print_ast(node->data.binop.left);
    printf(" AND ");
    print_ast(node->data.binop.right);
    printf(")");
    break;
  case NODE_OR:
    printf("(");
    print_ast(node->data.binop.left);
    printf(" OR ");
    print_ast(node->data.binop.right);
    printf(")");
    break;
  case NODE_IMPLIES:
    printf("(");
    print_ast(node->data.binop.left);
    printf(" IMPLIES ");
    print_ast(node->data.binop.right);
    printf(")");
    break;
  case NODE_PAREN:
    printf("(");
    print_ast(node->data.child);
    printf(")");
    break;
  }
}

void print_ast_latex(ast *node) {
  if (!node)
    return;
  switch (node->type) {
  case NODE_VAR:
    printf("%s", node->data.var_name);
    break;
  case NODE_NOT:
    printf("\\neg ");
    print_ast_latex(node->data.child);
    break;
  case NODE_AND:
    printf("(");
    print_ast_latex(node->data.binop.left);
    printf(" \\wedge ");
    print_ast_latex(node->data.binop.right);
    printf(")");
    break;
  case NODE_OR:
    printf("(");
    print_ast_latex(node->data.binop.left);
    printf(" \\vee ");
    print_ast_latex(node->data.binop.right);
    printf(")");
    break;
  case NODE_IMPLIES:
    printf("(");
    print_ast_latex(node->data.binop.left);
    printf(" \\rightarrow ");
    print_ast_latex(node->data.binop.right);
    printf(")");
    break;
  case NODE_PAREN:
    printf("(");
    print_ast_latex(node->data.child);
    printf(")");
    break;
  }
}

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

void process_input(CNF* cnf) {
    Assignment *assn = create_assignment();
    solve(cnf, assn);

    free_assignment(assn);
    free_cnf(cnf);
}

void yyerror(const char *msg) { fprintf(stderr, "error: %s\n", msg); }

int main(int argc, char **argv) {
  printf("> ");
  int result = yyparse();
  yylex_destroy();
  return result;
}
