#include "parser.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * parser.c - Recursive descent parser implementation
 */

/* Current token in the parsing process */
static int current_token;
static char* current_var;

/* Get the next token from the lexer */
static int next_token() {
    /* Free the previous token if any */
    if (current_token == TOKEN_VAR && current_var) {
        /* Don't free current_var directly as it points to yylval_atomo */
        /* free_current_token will handle this */
    }

    /* Free the previous token value */
    free_current_token();

    int token = yylex();

    /* Store variable name if token is VAR */
    if (token == TOKEN_VAR) {
        current_var = yylval_atomo;
    }

    current_token = token;
    return token;
}

/* Check if the current token matches the expected token */
static int match(int expected_token) {
    if (current_token == expected_token) {
        next_token();
        return 1;
    }
    return 0;
}

/* Parse an expression */
struct ast* parse_exp() {
    return parse_implies_exp();
}

/* Parse an implication expression */
struct ast* parse_implies_exp() {
    struct ast* left = parse_or_exp();
    if (!left) {
        return NULL; // Propagate error
    }

    if (current_token == TOKEN_IMPLIES) {
        next_token();
        struct ast* right = parse_implies_exp();
        if (!right) {
            free_ast(left); // Clean up left side on error
            return NULL;
        }
        return make_binary_node(NODE_IMPLIES, left, right);
    }

    return left;
}

/* Parse an OR expression */
struct ast* parse_or_exp() {
    struct ast* left = parse_and_exp();
    if (!left) {
        return NULL; // Propagate error
    }

    while (current_token == TOKEN_OR) {
        next_token();
        struct ast* right = parse_and_exp();
        if (!right) {
            free_ast(left); // Clean up left side on error
            return NULL;
        }
        struct ast* new_node = make_binary_node(NODE_OR, left, right);
        if (!new_node) {
            free_ast(left);
            free_ast(right);
            return NULL;
        }
        left = new_node;
    }

    return left;
}

/* Parse an AND expression */
struct ast* parse_and_exp() {
    struct ast* left = parse_not_exp();
    if (!left) {
        return NULL; // Propagate error
    }

    while (current_token == TOKEN_AND) {
        next_token();
        struct ast* right = parse_not_exp();
        if (!right) {
            free_ast(left); // Clean up left side on error
            return NULL;
        }
        struct ast* new_node = make_binary_node(NODE_AND, left, right);
        if (!new_node) {
            free_ast(left);
            free_ast(right);
            return NULL;
        }
        left = new_node;
    }

    return left;
}

/* Parse a NOT expression or a terminal */
struct ast* parse_not_exp() {
    if (current_token == TOKEN_NOT) {
        next_token();
        struct ast* operand = parse_not_exp();
        if (!operand) {
            return NULL; // Propagate error
        }
        return make_unary_node(NODE_NOT, operand);
    } else if (current_token == TOKEN_VAR) {
        char* var_name = strdup(current_var);
        if (!var_name) {
            syntax_error("Memory allocation error");
            return NULL;
        }
        next_token();
        return make_var_node(var_name);
    } else if (current_token == TOKEN_LPAREN) {
        next_token();
        struct ast* expr = parse_exp();
        if (!expr) {
            return NULL; // Propagate error
        }

        if (!match(TOKEN_RPAREN)) {
            syntax_error("Expected closing parenthesis");
            free_ast(expr);
            return NULL;
        }

        return expr;
    } else {
        syntax_error("Unexpected token in expression");
        return NULL;
    }
}

/* Parse the input for the solver */
struct ast* parse_input_linear() {
    /* Initialize error flag */
    syntax_error_occurred = 0;

    /* Get the first token */
    next_token();

    /* Check for the opening delimiter */
    if (!match(TOKEN_DELIM)) {
        syntax_error("Expected opening delimiter ($$)");
        return NULL;
    }

    /* Check for empty input */
    if (current_token == TOKEN_DELIM) {
        next_token();
        return NULL; /* Empty input */
    }

    /* Parse the expression */
    struct ast* ast_root = parse_exp();

    /* Check for the closing delimiter */
    if (!match(TOKEN_DELIM)) {
        syntax_error("Expected closing delimiter ($$)");
        if (ast_root) free_ast(ast_root);
        return NULL;
    }

    /* Check for syntax errors */
    if (syntax_error_occurred || !ast_root) {
        if (ast_root) free_ast(ast_root);
        return NULL;
    }

    return ast_root;
}
