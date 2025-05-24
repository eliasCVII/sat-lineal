#ifndef AST_H
#define AST_H

/*
 *
 * ast.h: Definiciones e interfaz para "Abstract Syntax Tree"
 *
 */


typedef enum {
  NODE_VAR,
  NODE_NOT,
  NODE_AND,
  NODE_OR,
  NODE_IMPLIES,
  NODE_PAREN
} NodeType;


/*
 *
 * Estructura del arbol AST
 * Representa un nodo en el árbol de sintaxis abstracta para expresiones lógicas.
 * - NODE_VAR: Variable proposicional (almacena nombre en var_name)
 * - NODE_NOT: Operador de negación (almacena operando en child)
 * - NODE_AND, NODE_OR, NODE_IMPLIES: Operadores binarios (almacenan operandos en binop)
 * - NODE_PAREN: Expresión entre paréntesis (almacena expresión en child)
 *
 */
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
} ast;


struct ast *make_var_node(char* name);
struct ast *make_unary_node(NodeType type, ast* child);
struct ast *make_binary_node(NodeType type, ast* l, ast* r);


void print_ast(ast* node);
void print_ast_latex(ast* node);


void free_ast(ast* node);

#endif
