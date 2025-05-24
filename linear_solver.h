#ifndef LINEAR_SOLVER_H
#define LINEAR_SOLVER_H

/**
 * linear_solver.h - Linear-time SAT solver using DAG representation
 */

#include "ast.h"

/* Constraint values for nodes */
typedef enum {
    UNCONSTRAINED = 0,
    TRUE = 1,
    FALSE = 2,
    CONFLICT = 3  /* Both TRUE and FALSE - indicates unsatisfiability */
} Constraint;

/* DAG node structure */
typedef struct DAGNode {
    NodeType type;           /* Type of node (VAR, NOT, AND, OR, IMPLIES) */
    Constraint constraint;   /* Current constraint (TRUE, FALSE, UNCONSTRAINED) */
    unsigned long hash;      /* Hash value for node sharing */

    union {
        char *var_name;      /* For variable nodes */
        struct DAGNode *child;  /* For unary operators (NOT) */
        struct {
            struct DAGNode *left;   /* Left operand for binary operators */
            struct DAGNode *right;  /* Right operand for binary operators */
        } binop;
    } data;

    /* For constraint propagation */
    struct DAGNode **parents;  /* Array of parent nodes */
    int parent_count;         /* Number of parent nodes */
    int parent_capacity;      /* Capacity of parents array */
} DAGNode;

/* Hash table for node sharing */
typedef struct NodeTable {
    DAGNode **nodes;         /* Array of node pointers */
    int size;                /* Current number of nodes */
    int capacity;            /* Current capacity of the table */
} NodeTable;

/* Worklist for constraint propagation */
typedef struct Worklist {
    DAGNode **nodes;         /* Array of node pointers */
    int size;                /* Current number of nodes */
    int capacity;            /* Current capacity of the list */
} Worklist;

/* Assignment structure */
typedef struct LinearAssignment {
    char **variables;        /* Array of variable names */
    int *values;             /* Array of values (0 or 1) */
    int size;                /* Number of variables */
} LinearAssignment;

/* DAG construction functions */
NodeTable *create_node_table();
DAGNode *create_var_node(NodeTable *table, char *name);
DAGNode *create_not_node(NodeTable *table, DAGNode *child);
DAGNode *create_and_node(NodeTable *table, DAGNode *left, DAGNode *right);
DAGNode *create_or_node(NodeTable *table, DAGNode *left, DAGNode *right);
DAGNode *create_implies_node(NodeTable *table, DAGNode *left, DAGNode *right);
DAGNode *ast_to_dag(NodeTable *table, ast *node);

/* Constraint propagation functions */
Worklist *create_worklist();
void add_to_worklist(Worklist *list, DAGNode *node);
DAGNode *remove_from_worklist(Worklist *list);
int propagate_constraints(DAGNode *root, Worklist *list);

/* Assignment functions */
LinearAssignment *create_linear_assignment();
void assign_linear_variable(LinearAssignment *assn, char *var, int value);
int get_linear_variable_value(LinearAssignment *assn, char *var);
void extract_assignment(DAGNode *root, LinearAssignment *assn);

/* Main solver function */
int linear_solve(ast *formula, LinearAssignment *assn);

/* Memory management */
void add_parent(DAGNode *child, DAGNode *parent);
void free_dag_node(DAGNode *node);
void free_node_table(NodeTable *table);
void free_worklist(Worklist *list);
void free_linear_assignment(LinearAssignment *assn);

#endif /* LINEAR_SOLVER_H */