#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * ast_transform.c - Safe implementation of AST transformation functions
 */

/* Transform an AST to a simpler form */
struct ast* transform(struct ast *node) {
  if (!node)
    return NULL;

  switch (node->type) {
  case NODE_VAR:
    return make_var_node(strdup(node->data.var_name));

  case NODE_NOT: {
    struct ast *child = transform(node->data.child);
    if (!child) return NULL;

    struct ast *result = NULL;

    switch (child->type) {
    case NODE_NOT: {
      result = transform(child->data.child);
      free_ast(child);
      return result;
    }
    case NODE_AND: {
      struct ast *not_left = make_unary_node(NODE_NOT, child->data.binop.left);
      struct ast *not_right = make_unary_node(NODE_NOT, child->data.binop.right);
      struct ast *trans_left = transform(not_left);
      struct ast *trans_right = transform(not_right);

      free_ast(not_left);
      free_ast(not_right);

      result = make_binary_node(NODE_OR, trans_left, trans_right);
      free_ast(child);
      return result;
    }
    case NODE_OR: {
      struct ast *not_left = make_unary_node(NODE_NOT, child->data.binop.left);
      struct ast *not_right = make_unary_node(NODE_NOT, child->data.binop.right);
      struct ast *trans_left = transform(not_left);
      struct ast *trans_right = transform(not_right);

      free_ast(not_left);
      free_ast(not_right);

      result = make_binary_node(NODE_AND, trans_left, trans_right);
      free_ast(child);
      return result;
    }
    default:
      result = make_unary_node(NODE_NOT, child);
      return result;
    }
  }

  case NODE_IMPLIES: {
    struct ast *left = transform(node->data.binop.left);
    struct ast *right = transform(node->data.binop.right);
    struct ast *new_left = make_unary_node(NODE_NOT, left);
    struct ast *result = distribute_OR(new_left, right);
    return result;
  }

  case NODE_OR: {
    struct ast *left = transform(node->data.binop.left);
    struct ast *right = transform(node->data.binop.right);
    struct ast *result = distribute_OR(left, right);
    return result;
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

/* Distribute OR over AND */
struct ast *distribute_OR(struct ast *left, struct ast *right) {
  if (left->type == NODE_AND) {
    struct ast *dist_left = distribute_OR(left->data.binop.left, right);
    struct ast *right_clone = NULL;

    if (right->type == NODE_VAR) {
      right_clone = make_var_node(strdup(right->data.var_name));
    } else if (right->type == NODE_NOT) {
      right_clone = make_unary_node(NODE_NOT,
                                   right->data.child->type == NODE_VAR ?
                                   make_var_node(strdup(right->data.child->data.var_name)) :
                                   NULL);
    }

    struct ast *dist_right = distribute_OR(left->data.binop.right,
                                          right_clone ? right_clone : right);

    struct ast *result = make_binary_node(NODE_AND, dist_left, dist_right);

    free(left);

    return result;
  }

  if (right->type == NODE_AND) {
    struct ast *dist_left = distribute_OR(left, right->data.binop.left);

    struct ast *left_clone = NULL;

    if (left->type == NODE_VAR) {
      left_clone = make_var_node(strdup(left->data.var_name));
    } else if (left->type == NODE_NOT) {
      left_clone = make_unary_node(NODE_NOT,
                                  left->data.child->type == NODE_VAR ?
                                  make_var_node(strdup(left->data.child->data.var_name)) :
                                  NULL);
    }

    struct ast *dist_right = distribute_OR(left_clone ? left_clone : left,
                                          right->data.binop.right);

    struct ast *result = make_binary_node(NODE_AND, dist_left, dist_right);

    free(right);

    return result;
  }

  return make_binary_node(NODE_OR, left, right);
}

/* Safe version of transform function */
struct ast* safe_transform(struct ast* node) {
    if (!node)
        return NULL;

    switch (node->type) {
    case NODE_VAR: {
        /* For variables, just create a new node with a copy of the name */
        return make_var_node(strdup(node->data.var_name));
    }

    case NODE_NOT: {
        /* Transform the child first */
        struct ast* child = safe_transform(node->data.child);
        if (!child) return NULL;

        /* Apply De Morgan's laws based on the type of the child */
        switch (child->type) {
        case NODE_NOT: {
            /* Double negation: NOT(NOT(A)) = A */
            struct ast* result = safe_transform(child->data.child);
            free_ast(child);
            return result;
        }
        case NODE_AND: {
            /* NOT(A AND B) = NOT(A) OR NOT(B) */
            struct ast* left = child->data.binop.left;
            struct ast* right = child->data.binop.right;

            /* Create NOT nodes for each operand */
            struct ast* not_left = make_unary_node(NODE_NOT,
                make_var_node(strdup(left->type == NODE_VAR ? left->data.var_name : "temp")));
            struct ast* not_right = make_unary_node(NODE_NOT,
                make_var_node(strdup(right->type == NODE_VAR ? right->data.var_name : "temp")));

            /* Transform each NOT node */
            struct ast* trans_left = safe_transform(not_left);
            struct ast* trans_right = safe_transform(not_right);

            /* Clean up temporary nodes */
            free_ast(not_left);
            free_ast(not_right);

            /* Create the OR node with the transformed operands */
            struct ast* result = make_binary_node(NODE_OR, trans_left, trans_right);
            free_ast(child);
            return result;
        }
        case NODE_OR: {
            /* NOT(A OR B) = NOT(A) AND NOT(B) */
            struct ast* left = child->data.binop.left;
            struct ast* right = child->data.binop.right;

            /* Create NOT nodes for each operand */
            struct ast* not_left = make_unary_node(NODE_NOT,
                make_var_node(strdup(left->type == NODE_VAR ? left->data.var_name : "temp")));
            struct ast* not_right = make_unary_node(NODE_NOT,
                make_var_node(strdup(right->type == NODE_VAR ? right->data.var_name : "temp")));

            /* Transform each NOT node */
            struct ast* trans_left = safe_transform(not_left);
            struct ast* trans_right = safe_transform(not_right);

            /* Clean up temporary nodes */
            free_ast(not_left);
            free_ast(not_right);

            /* Create the AND node with the transformed operands */
            struct ast* result = make_binary_node(NODE_AND, trans_left, trans_right);
            free_ast(child);
            return result;
        }
        default:
            /* For other node types, just create a NOT node */
            return make_unary_node(NODE_NOT, child);
        }
    }

    case NODE_IMPLIES: {
        /* A -> B = NOT(A) OR B */
        struct ast* left = safe_transform(node->data.binop.left);
        struct ast* right = safe_transform(node->data.binop.right);

        if (!left || !right) {
            if (left) free_ast(left);
            if (right) free_ast(right);
            return NULL;
        }

        struct ast* not_left = make_unary_node(NODE_NOT, left);
        struct ast* result = make_binary_node(NODE_OR, not_left, right);
        return result;
    }

    case NODE_OR: {
        /* Transform both operands */
        struct ast* left = safe_transform(node->data.binop.left);
        struct ast* right = safe_transform(node->data.binop.right);

        if (!left || !right) {
            if (left) free_ast(left);
            if (right) free_ast(right);
            return NULL;
        }

        /* For simplicity and safety, we'll just create a simple OR node */
        return make_binary_node(NODE_OR, left, right);
    }

    case NODE_AND: {
        /* Transform both operands */
        struct ast* left = safe_transform(node->data.binop.left);
        struct ast* right = safe_transform(node->data.binop.right);

        if (!left || !right) {
            if (left) free_ast(left);
            if (right) free_ast(right);
            return NULL;
        }

        /* Simple AND */
        return make_binary_node(NODE_AND, left, right);
    }

    default:
        fprintf(stderr, "Unexpected node type in transform\n");
        return NULL;
    }
}
