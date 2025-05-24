#ifndef LINEAR_SOLVER_H
#define LINEAR_SOLVER_H

/*
 *
 * linear_solver.h: Interfaz para el solucionador de SAT lineal
 *
 */

#include "ast.h"

/* 
 * 
 * Valores de las restricciones para los nodos
 *
 */
typedef enum {
    UNCONSTRAINED = 0,
    TRUE = 1,
    FALSE = 2,
    CONFLICT = 3  // Ambos son TRUE y FALSE -> Contradiccion
} Constraint;

typedef struct DAGNode {
    NodeType type;
    Constraint constraint;
    unsigned long hash; // Valor de hash para compartir nodos
    union {
        char *var_name;
        struct DAGNode *child;
        struct {
            struct DAGNode *left;
            struct DAGNode *right;
        } binop;
    } data;
    struct DAGNode **parents; // Arreglo de nodos padres
    int parent_count;
    int parent_capacity;
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

/*
 *
 * Funciones para construccion del grafo aciclico dirigido (DAG)
 *
 */
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

/*
 *
 */
LinearAssignment *create_linear_assignment();
void assign_linear_variable(LinearAssignment *assn, char *var, int value);
int get_linear_variable_value(LinearAssignment *assn, char *var);
void extract_assignment(DAGNode *root, LinearAssignment *assn);


int linear_solve(ast *formula, LinearAssignment *assn);

/*
 *
 * Manejo de memoria
 *
 */
void add_parent(DAGNode *child, DAGNode *parent);
void free_dag_node(DAGNode *node);
void free_node_table(NodeTable *table);
void free_worklist(Worklist *list);
void free_linear_assignment(LinearAssignment *assn);

#endif