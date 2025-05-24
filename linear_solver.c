#include "linear_solver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * linear_solver.c - Implementation of a linear-time SAT solver using DAG representation
 */

#define INITIAL_CAPACITY 16
#define HASH_MULTIPLIER 31

/* Forward declarations of static functions */
static unsigned long hash_string(const char *str);
static unsigned long hash_node(DAGNode *node);
static DAGNode *find_or_add_node(NodeTable *table, DAGNode *node);
static int set_constraint(DAGNode *node, Constraint constraint, Worklist *list);

/* Hash function for strings */
static unsigned long hash_string(const char *str) {
    unsigned long hash = 0;
    char current;

    while (*str) {
        current = *str;
        str = str + 1;
        hash = hash * HASH_MULTIPLIER + current;
    }
    return hash;
}

/* Hash function for nodes */
static unsigned long hash_node(DAGNode *node) {
    unsigned long hash = node->type;

    switch (node->type) {
        case NODE_VAR:
            return hash_string(node->data.var_name);
        case NODE_NOT:
            return hash * 31 + node->data.child->hash;
        case NODE_AND:
        case NODE_OR:
        case NODE_IMPLIES:
            return hash * 31 + node->data.binop.left->hash * 17 + node->data.binop.right->hash;
        default:
            return hash;
    }
}

/* Create a new node table */
NodeTable *create_node_table() {
    NodeTable *table = calloc(1, sizeof(NodeTable));
    table->capacity = INITIAL_CAPACITY;
    table->size = 0;
    table->nodes = calloc(table->capacity, sizeof(DAGNode*));
    return table;
}

/* Add a parent to a node */
void add_parent(DAGNode *child, DAGNode *parent) {
    if (!child || !parent) return;

    if (child->parent_count >= child->parent_capacity) {
        child->parent_capacity = child->parent_capacity ? child->parent_capacity * 2 : 4;
        child->parents = realloc(child->parents, child->parent_capacity * sizeof(DAGNode*));
    }

    child->parents[child->parent_count] = parent;
    child->parent_count = child->parent_count + 1;
}

/* Find a node in the table or add it if not found */
static DAGNode *find_or_add_node(NodeTable *table, DAGNode *node) {
    unsigned long hash;
    int i;
    DAGNode *existing;

    hash = hash_node(node);
    node->hash = hash;

    /* Check if node already exists */
    for (i = 0; i < table->size; i = i + 1) {
        existing = table->nodes[i];

        if (existing->hash == hash && existing->type == node->type) {
            switch (node->type) {
                case NODE_VAR:
                    if (strcmp(existing->data.var_name, node->data.var_name) == 0) {
                        free(node->data.var_name);
                        free(node);
                        return existing;
                    }
                    break;
                case NODE_NOT:
                    if (existing->data.child == node->data.child) {
                        free(node);
                        return existing;
                    }
                    break;
                case NODE_AND:
                case NODE_OR:
                case NODE_IMPLIES:
                    if (existing->data.binop.left == node->data.binop.left &&
                        existing->data.binop.right == node->data.binop.right) {
                        free(node);
                        return existing;
                    }
                    break;
                default:
                    break;
            }
        }
    }

    /* Resize table if needed */
    if (table->size >= table->capacity) {
        table->capacity = table->capacity * 2;
        table->nodes = realloc(table->nodes, table->capacity * sizeof(DAGNode*));
    }

    /* Add new node */
    table->nodes[table->size] = node;
    table->size = table->size + 1;
    return node;
}

/* Create a variable node */
DAGNode *create_var_node(NodeTable *table, char *name) {
    DAGNode *node = calloc(1, sizeof(DAGNode));
    node->type = NODE_VAR;
    node->constraint = UNCONSTRAINED;
    node->data.var_name = strdup(name);
    node->parents = NULL;
    node->parent_count = 0;
    node->parent_capacity = 0;

    return find_or_add_node(table, node);
}

/* Create a NOT node */
DAGNode *create_not_node(NodeTable *table, DAGNode *child) {
    DAGNode *node;
    DAGNode *result;

    node = calloc(1, sizeof(DAGNode));
    node->type = NODE_NOT;
    node->constraint = UNCONSTRAINED;
    node->data.child = child;
    node->parents = NULL;
    node->parent_count = 0;
    node->parent_capacity = 0;

    result = find_or_add_node(table, node);

    /* Add parent relationship if this is a new node */
    if (result == node) {
        add_parent(child, node);
    }

    return result;
}

/* Create an AND node */
DAGNode *create_and_node(NodeTable *table, DAGNode *left, DAGNode *right) {
    DAGNode *node;
    DAGNode *result;

    node = calloc(1, sizeof(DAGNode));
    node->type = NODE_AND;
    node->constraint = UNCONSTRAINED;
    node->data.binop.left = left;
    node->data.binop.right = right;
    node->parents = NULL;
    node->parent_count = 0;
    node->parent_capacity = 0;

    result = find_or_add_node(table, node);

    /* Add parent relationships if this is a new node */
    if (result == node) {
        add_parent(left, node);
        add_parent(right, node);
    }

    return result;
}

/* Create an OR node */
DAGNode *create_or_node(NodeTable *table, DAGNode *left, DAGNode *right) {
    DAGNode *node;
    DAGNode *result;

    node = calloc(1, sizeof(DAGNode));
    node->type = NODE_OR;
    node->constraint = UNCONSTRAINED;
    node->data.binop.left = left;
    node->data.binop.right = right;
    node->parents = NULL;
    node->parent_count = 0;
    node->parent_capacity = 0;

    result = find_or_add_node(table, node);

    /* Add parent relationships if this is a new node */
    if (result == node) {
        add_parent(left, node);
        add_parent(right, node);
    }

    return result;
}

/* Create an IMPLIES node */
DAGNode *create_implies_node(NodeTable *table, DAGNode *left, DAGNode *right) {
    DAGNode *not_left;

    /* p → q is equivalent to ¬p ∨ q */
    not_left = create_not_node(table, left);
    return create_or_node(table, not_left, right);
}

/* Convert AST to DAG */
DAGNode *ast_to_dag(NodeTable *table, ast *node) {
    if (!node) return NULL;

    switch (node->type) {
        case NODE_VAR:
            return create_var_node(table, node->data.var_name);
        case NODE_NOT:
            return create_not_node(table, ast_to_dag(table, node->data.child));
        case NODE_AND:
            return create_and_node(table,
                ast_to_dag(table, node->data.binop.left),
                ast_to_dag(table, node->data.binop.right));
        case NODE_OR:
            return create_or_node(table,
                ast_to_dag(table, node->data.binop.left),
                ast_to_dag(table, node->data.binop.right));
        case NODE_IMPLIES:
            return create_implies_node(table,
                ast_to_dag(table, node->data.binop.left),
                ast_to_dag(table, node->data.binop.right));
        case NODE_PAREN:
            return ast_to_dag(table, node->data.child);
        default:
            fprintf(stderr, "Unknown node type in AST\n");
            return NULL;
    }
}

/* Create a worklist for constraint propagation */
Worklist *create_worklist() {
    Worklist *list = calloc(1, sizeof(Worklist));
    list->capacity = INITIAL_CAPACITY;
    list->size = 0;
    list->nodes = calloc(list->capacity, sizeof(DAGNode*));
    return list;
}

/* Add a node to the worklist */
void add_to_worklist(Worklist *list, DAGNode *node) {
    if (!node) return;

    /* Resize if needed */
    if (list->size >= list->capacity) {
        list->capacity = list->capacity * 2;
        list->nodes = realloc(list->nodes, list->capacity * sizeof(DAGNode*));
    }

    list->nodes[list->size] = node;
    list->size = list->size + 1;
}

/* Remove a node from the worklist */
DAGNode *remove_from_worklist(Worklist *list) {
    if (list->size == 0) return NULL;
    list->size = list->size - 1;
    return list->nodes[list->size];
}

/* Set a constraint on a node and add it to the worklist if changed */
static int set_constraint(DAGNode *node, Constraint constraint, Worklist *list) {
    if (node->constraint == constraint) {
        return 1; /* No change */
    }

    if (node->constraint == UNCONSTRAINED) {
        node->constraint = constraint;
        add_to_worklist(list, node);
        return 1;
    } else if (node->constraint != constraint) {
        node->constraint = CONFLICT;
        return 0; /* Conflict detected */
    }

    return 1;
}

/* Propagate constraints through the DAG */
int propagate_constraints(DAGNode *root, Worklist *list) {
    DAGNode *node;
    int result;

    /* Start by setting the root to TRUE */
    if (!set_constraint(root, TRUE, list)) {
        return 0; /* Conflict at root */
    }

    /* Process nodes in the worklist */
    while (list->size > 0) {
        node = remove_from_worklist(list);

        if (node->constraint == CONFLICT) {
            return 0; /* Conflict detected */
        }

        switch (node->type) {
            case NODE_VAR:
                /* Variables don't propagate constraints */
                break;

            case NODE_NOT:
                /* NOT propagates the opposite constraint */
                if (node->constraint == TRUE) {
                    if (!set_constraint(node->data.child, FALSE, list)) {
                        return 0;
                    }
                } else if (node->constraint == FALSE) {
                    if (!set_constraint(node->data.child, TRUE, list)) {
                        return 0;
                    }
                }
                break;

            case NODE_AND:
                if (node->constraint == TRUE) {
                    /* If AND is TRUE, both children must be TRUE */
                    if (!set_constraint(node->data.binop.left, TRUE, list) ||
                        !set_constraint(node->data.binop.right, TRUE, list)) {
                        return 0;
                    }
                } else if (node->constraint == FALSE) {
                    /* If AND is FALSE, at least one child must be FALSE */
                    /* This is where the linear solver is incomplete */
                    /* We can't deterministically decide which child should be FALSE */
                    /* For a complete solver, we would need backtracking here */

                    /* Check if one child is already constrained */
                    if (node->data.binop.left->constraint == TRUE) {
                        if (!set_constraint(node->data.binop.right, FALSE, list)) {
                            return 0;
                        }
                    } else if (node->data.binop.right->constraint == TRUE) {
                        if (!set_constraint(node->data.binop.left, FALSE, list)) {
                            return 0;
                        }
                    }
                    /* Otherwise, we can't deterministically propagate */
                }
                break;

            case NODE_OR:
                if (node->constraint == FALSE) {
                    /* If OR is FALSE, both children must be FALSE */
                    if (!set_constraint(node->data.binop.left, FALSE, list) ||
                        !set_constraint(node->data.binop.right, FALSE, list)) {
                        return 0;
                    }
                } else if (node->constraint == TRUE) {
                    /* If OR is TRUE, at least one child must be TRUE */
                    /* This is where the linear solver is incomplete */
                    /* We can't deterministically decide which child should be TRUE */

                    /* Check if one child is already constrained */
                    if (node->data.binop.left->constraint == FALSE) {
                        if (!set_constraint(node->data.binop.right, TRUE, list)) {
                            return 0;
                        }
                    } else if (node->data.binop.right->constraint == FALSE) {
                        if (!set_constraint(node->data.binop.left, TRUE, list)) {
                            return 0;
                        }
                    }
                    /* Otherwise, we can't deterministically propagate */
                }
                break;

            default:
                /* Other node types should have been transformed */
                break;
        }
    }

    return 1; /* No conflicts found */
}

/* Create a new assignment */
LinearAssignment *create_linear_assignment() {
    LinearAssignment *assn = calloc(1, sizeof(LinearAssignment));
    assn->variables = NULL;
    assn->values = NULL;
    assn->size = 0;
    return assn;
}

/* Assign a value to a variable */
void assign_linear_variable(LinearAssignment *assn, char *var, int value) {
    int i;

    for (i = 0; i < assn->size; i = i + 1) {
        if (strcmp(assn->variables[i], var) == 0) {
            assn->values[i] = value;
            return;
        }
    }

    assn->size = assn->size + 1;
    assn->variables = realloc(assn->variables, assn->size * sizeof(char *));
    assn->values = realloc(assn->values, assn->size * sizeof(int));
    assn->variables[assn->size - 1] = strdup(var);
    assn->values[assn->size - 1] = value;
}

/* Get the value of a variable (-1 if unassigned) */
int get_linear_variable_value(LinearAssignment *assn, char *var) {
    int i;

    for (i = 0; i < assn->size; i = i + 1) {
        if (strcmp(assn->variables[i], var) == 0) {
            return assn->values[i];
        }
    }
    return -1;
}

/* Extract variable assignments from the DAG */
void extract_assignment(DAGNode *root, LinearAssignment *assn) {
    int i;

    /* Traverse the DAG and extract variable assignments */
    for (i = 0; i < root->parent_count; i = i + 1) {
        extract_assignment(root->parents[i], assn);
    }

    if (root->type == NODE_VAR) {
        if (root->constraint == TRUE) {
            assign_linear_variable(assn, root->data.var_name, 1);
        } else if (root->constraint == FALSE) {
            assign_linear_variable(assn, root->data.var_name, 0);
        } else {
            /* For unconstrained variables, assign arbitrary value (true) */
            assign_linear_variable(assn, root->data.var_name, 1);
        }
    }
}

/* Main solver function */
int linear_solve(ast *formula, LinearAssignment *assn) {
    /* Create node table for DAG construction */
    NodeTable *table;
    DAGNode *root;
    Worklist *list;
    int result;

    table = create_node_table();

    /* Convert AST to DAG */
    root = ast_to_dag(table, formula);

    /* Create worklist for constraint propagation */
    list = create_worklist();

    /* Propagate constraints */
    result = propagate_constraints(root, list);

    /* Extract variable assignments if satisfiable */
    if (result) {
        extract_assignment(root, assn);
    }

    /* Free resources */
    free_worklist(list);
    free_node_table(table);

    return result;
}

/* Free a DAG node */
void free_dag_node(DAGNode *node) {
    if (!node) return;

    if (node->type == NODE_VAR) {
        free(node->data.var_name);
    }

    free(node->parents);
    free(node);
}

/* Free a node table */
void free_node_table(NodeTable *table) {
    int i;

    if (!table) return;

    for (i = 0; i < table->size; i = i + 1) {
        free_dag_node(table->nodes[i]);
    }

    free(table->nodes);
    free(table);
}

/* Free a worklist */
void free_worklist(Worklist *list) {
    if (!list) return;

    free(list->nodes);
    free(list);
}

/* Free an assignment */
void free_linear_assignment(LinearAssignment *assn) {
    int i;

    if (!assn) return;

    for (i = 0; i < assn->size; i = i + 1) {
        free(assn->variables[i]);
    }

    free(assn->variables);
    free(assn->values);
    free(assn);
}