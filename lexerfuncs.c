#include "lexerast.h"
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

struct ast *translate(struct ast *node) {
  if (!node) {
    return NULL;
  }

  switch (node->type) {
  case NODE_VAR:
    return make_var_node(strdup(node->data.var_name));

  case NODE_NOT: {
    struct ast *child = translate(node->data.child);
    switch (child->type) {
    case NODE_NOT:
      return translate(child->data.child);
    case NODE_AND:
      return make_unary_node(
          NODE_NOT,
          make_binary_node(
              NODE_AND,
              make_unary_node(NODE_NOT, translate(child->data.binop.left)),
              make_unary_node(NODE_NOT, translate(child->data.binop.right))));
    case NODE_OR:
      return make_binary_node(
          NODE_AND, make_unary_node(NODE_NOT, translate(child->data.binop.left)),
          make_unary_node(NODE_NOT, translate(child->data.binop.right)));
    default:
      return make_unary_node(NODE_NOT, child);
    }
  }

  case NODE_IMPLIES: {
    struct ast *left = translate(node->data.binop.left);
    struct ast *right = translate(node->data.binop.right);
    return make_unary_node(
        NODE_NOT,
        make_binary_node(NODE_AND, left, make_unary_node(NODE_NOT, right)));
  }

  case NODE_OR: {
    struct ast *left = translate(node->data.binop.left);
    struct ast *right = translate(node->data.binop.right);
    return make_unary_node(
        NODE_NOT, make_binary_node(NODE_AND, make_unary_node(NODE_NOT, left),
                                   make_unary_node(NODE_NOT, right)));
  }

  case NODE_AND: {
    struct ast *left = translate(node->data.binop.left);
    struct ast *right = translate(node->data.binop.right);
    return make_binary_node(NODE_AND, left, right);
  }

  default:
    return NULL;
  }
}

struct ast* demorgan(struct ast* node) {
    if (!node) return NULL;

    switch (node->type) {
        case NODE_VAR:
            // Create a copy of the variable node
            return make_var_node(strdup(node->data.var_name));

        case NODE_NOT:
            if (node->data.child->type == NODE_AND) {
                // ¬(A ∧ B) → ¬A ∨ ¬B
                struct ast* left = demorgan(node->data.child->data.binop.left);
                struct ast* right = demorgan(node->data.child->data.binop.right);
                struct ast* new_left = make_unary_node(NODE_NOT, left);
                struct ast* new_right = make_unary_node(NODE_NOT, right);
                return make_binary_node(NODE_OR, new_left, new_right);
            }
            else if (node->data.child->type == NODE_OR) {
                // ¬(A ∨ B) → ¬A ∧ ¬B
                struct ast* left = demorgan(node->data.child->data.binop.left);
                struct ast* right = demorgan(node->data.child->data.binop.right);
                struct ast* new_left = make_unary_node(NODE_NOT, left);
                struct ast* new_right = make_unary_node(NODE_NOT, right);
                return make_binary_node(NODE_AND, new_left, new_right);
            }
            else if (node->data.child->type == NODE_NOT) {
                // ¬¬A → A (eliminate double negation, create a copy)
                struct ast* child = demorgan(node->data.child->data.child);
                return child; // Assuming child is already a deep copy
            }
            else {
                // ¬A where A is a literal; create new node
                struct ast* child = demorgan(node->data.child);
                return make_unary_node(NODE_NOT, child);
            }

        case NODE_AND:
        case NODE_OR:
            return make_binary_node(
                node->type,
                demorgan(node->data.binop.left),
                demorgan(node->data.binop.right)
            );

        default:
            fprintf(stderr, "Unexpected node type in demorgan\n");
            return NULL;
    }
}

struct ast* distribute_OR(struct ast* left, struct ast* right) {
    if (left->type == NODE_AND) {
        // A ∨ (B ∧ C) → (A ∨ B) ∧ (A ∨ C)
        return make_binary_node(
            NODE_AND,
            distribute_OR(left->data.binop.left, right),
            distribute_OR(left->data.binop.right, right)
        );
    }
    if (right->type == NODE_AND) {
        // (B ∧ C) ∨ A → (B ∨ A) ∧ (C ∨ A)
        return make_binary_node(
            NODE_AND,
            distribute_OR(left, right->data.binop.left),
            distribute_OR(left, right->data.binop.right)
        );
    }
    // Base case: both literals or ORs already handled
    return make_binary_node(NODE_OR, left, right);
}

struct ast* to_cnf(struct ast* node) {
    if (!node) return NULL;

    switch (node->type) {
        case NODE_VAR:
        case NODE_NOT:
            return node;  // Literals remain unchanged

        case NODE_AND:
            return make_binary_node(
                NODE_AND,
                to_cnf(node->data.binop.left),
                to_cnf(node->data.binop.right)
            );

        case NODE_OR:
            return distribute_OR(
                to_cnf(node->data.binop.left),
                to_cnf(node->data.binop.right)
            );

        default:
            fprintf(stderr, "Unexpected node type in to_CNF\n");
            return NULL;
    }
}

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
    if (!node) return;
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

void yyerror(const char *msg) { fprintf(stderr, "error: %s\n", msg); }

int main(int argc, char **argv) {
  printf("> ");
  int result =  yyparse();
  yylex_destroy();
  return result;
}
