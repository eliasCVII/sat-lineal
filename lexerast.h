#ifndef AST_H
#define AST_H

extern int yylex();
extern int yyparse();
void yyerror(const char *msg);
int yylex_destroy();

typedef enum {
  NODE_VAR,
  NODE_NOT,
  NODE_AND,
  NODE_OR,
  NODE_IMPLIES,
  NODE_PAREN
} NodeType;

typedef struct ast {
  NodeType type;
  union {
    char *var_name;
    struct ast *child;
    struct {
      struct ast *left;
      struct ast *right;
    } binop;
  } data;
}ast;

// Construye AST
struct ast *make_var_node(char* name);
struct ast *make_unary_node(NodeType type, ast* child);
struct ast *make_binary_node(NodeType type, ast* l, ast* r);

// Double Negation
/* struct ast* simplify_negations(struct ast* node); */

// Traduce AST
struct ast* to_nnf(struct ast* node);

/* struct ast *to_simplified_nnf(struct ast* node); */

// Libera AST
void free_ast(ast* node);

// Print
void print_ast(ast* node);

// Print Latex
void print_ast_latex(ast* node);

#endif
