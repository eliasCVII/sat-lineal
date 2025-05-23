#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * ast.c - Implementation of AST operations
 */

/* Create a variable node */
struct ast *make_var_node(char *name) {
  ast *node = malloc(sizeof(ast));
  node->type = NODE_VAR;
  node->data.var_name = name;
  return node;
}

/* Create a unary operator node (NOT, parentheses) */
struct ast *make_unary_node(NodeType type, ast *child) {
  ast *node = malloc(sizeof(ast));
  node->type = type;
  node->data.child = child;
  return node;
}

/* Create a binary operator node (AND, OR, IMPLIES) */
struct ast *make_binary_node(NodeType type, ast *l, ast *r) {
  ast *node = malloc(sizeof(ast));
  node->type = type;
  node->data.binop.left = l;
  node->data.binop.right = r;
  return node;
}

/* Free an AST node and all its children */
void free_ast(ast *node) {
  if (!node)
    return;
  switch (node->type) {
  case NODE_VAR:
    free(node->data.var_name);
    break;
  case NODE_NOT:
  case NODE_PAREN:
    free_ast(node->data.child);
    break;
  case NODE_AND:
  case NODE_OR:
  case NODE_IMPLIES:
    free_ast(node->data.binop.left);
    free_ast(node->data.binop.right);
    break;
  }
  free(node);
}

/* Print an AST in text format */
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

/* Print an AST in LaTeX format */
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
