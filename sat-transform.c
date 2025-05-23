/**
 * sat-transform.c - Safe implementation of AST transformation functions
 *
 * This file provides safer versions of the transform and related functions
 * to avoid memory management issues.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sat-header.h"

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
