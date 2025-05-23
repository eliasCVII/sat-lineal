#ifndef AST_H
#define AST_H

/**
 * ast.h - Abstract Syntax Tree type definitions and interface
 */

/* Node types for the AST */
typedef enum {
  NODE_VAR,    /* Variable node */
  NODE_NOT,    /* Negation node */
  NODE_AND,    /* AND node */
  NODE_OR,     /* OR node */
  NODE_IMPLIES, /* Implication node */
  NODE_PAREN   /* Parenthesis node (for parsing) */
} NodeType;

/* AST node structure */
typedef struct ast {
  NodeType type;
  union {
    char *var_name;        /* For variable nodes */
    struct ast *child;     /* For unary operators (NOT, parentheses) */
    struct {
      struct ast *left;    /* Left operand for binary operators */
      struct ast *right;   /* Right operand for binary operators */
    } binop;
  } data;
} ast;

/* AST construction functions */
struct ast *make_var_node(char* name);
struct ast *make_unary_node(NodeType type, ast* child);
struct ast *make_binary_node(NodeType type, ast* l, ast* r);

/* AST transformation functions - removed for linear solver */

/* AST printing functions */
void print_ast(ast* node);
void print_ast_latex(ast* node);

/* Memory management */
void free_ast(ast* node);

#endif /* AST_H */
