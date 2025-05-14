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

struct ast *to_nnf(struct ast *node) {
  if (!node) {
    return NULL;
  }

  switch (node->type) {
  case NODE_VAR:
    return make_var_node(strdup(node->data.var_name));

  case NODE_NOT: {
    struct ast *child = to_nnf(node->data.child);
    switch (child->type) {
    case NODE_NOT:
      return to_nnf(child->data.child);
    case NODE_AND:
      return make_unary_node(
          NODE_NOT,
          make_binary_node(
              NODE_AND,
              make_unary_node(NODE_NOT, to_nnf(child->data.binop.left)),
              make_unary_node(NODE_NOT, to_nnf(child->data.binop.right))));
    case NODE_OR:
      return make_binary_node(
          NODE_AND, make_unary_node(NODE_NOT, to_nnf(child->data.binop.left)),
          make_unary_node(NODE_NOT, to_nnf(child->data.binop.right)));
    default:
      return make_unary_node(NODE_NOT, child);
    }
  }

  case NODE_IMPLIES: {
    struct ast *left = to_nnf(node->data.binop.left);
    struct ast *right = to_nnf(node->data.binop.right);
    return make_unary_node(
        NODE_NOT,
        make_binary_node(NODE_AND, left, make_unary_node(NODE_NOT, right)));
  }

  case NODE_OR: {
    struct ast *left = to_nnf(node->data.binop.left);
    struct ast *right = to_nnf(node->data.binop.right);
    return make_unary_node(
        NODE_NOT, make_binary_node(NODE_AND, make_unary_node(NODE_NOT, left),
                                   make_unary_node(NODE_NOT, right)));
  }

  case NODE_AND: {
    struct ast *left = to_nnf(node->data.binop.left);
    struct ast *right = to_nnf(node->data.binop.right);
    return make_binary_node(NODE_AND, left, right);
  }

  default:
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
