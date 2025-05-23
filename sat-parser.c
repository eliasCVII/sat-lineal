/**
 * sat-parser.c - Pure C implementation of the SAT parser
 *
 * This file replaces the Bison/Yacc-based parser (sat-parser.y) with a
 * hand-written recursive descent parser in pure C. It maintains compatibility
 * with the existing Flex-based lexer (sat-lex.l).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sat-header.h"

/* Token definitions - must match those in the original Bison grammar */
#define TOKEN_DELIM    1
#define TOKEN_IMPLIES  2
#define TOKEN_OR       3
#define TOKEN_AND      4
#define TOKEN_NOT      5
#define TOKEN_LPAREN   6
#define TOKEN_RPAREN   7
#define TOKEN_VAR      8
#define TOKEN_ERROR    0

/* External variables from the lexer */
extern int yylex();
extern char* yylval_atomo;  /* We'll define this in the lexer */
extern int syntax_error_occurred;
extern void free_current_token();  /* Function to free the current token */

/* External function from sat-transform.c */
extern struct ast* safe_transform(struct ast* node); /* Used for AST transformation */

/* Forward declarations for recursive descent parser functions */
struct ast* parse_exp();
struct ast* parse_implies_exp();
struct ast* parse_or_exp();
struct ast* parse_and_exp();
struct ast* parse_not_exp();

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

/* Report a syntax error */
static void syntax_error(const char* message) {
    yyerror(message);
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

/* Parse the input according to the grammar */
int parse_input() {
    /* Initialize error flag */
    syntax_error_occurred = 0;

    /* Get the first token */
    next_token();

    /* Check for the opening delimiter */
    if (!match(TOKEN_DELIM)) {
        syntax_error("Expected opening delimiter ($$)");
        return 1;
    }

    /* Check for empty input */
    if (current_token == TOKEN_DELIM) {
        next_token();

        /* Empty input - create an empty CNF */
        CNF* empty = malloc(sizeof(CNF));
        if (!empty) {
            fprintf(stderr, "Memory allocation error\n");
            printf("NO-SOLUTION\n");
            return 1;
        }
        empty->clauses = NULL;
        empty->count = 0;
        process_input(empty);

        return 0;
    }

    /* Parse the expression */
    struct ast* ast_root = parse_exp();

    /* Check for the closing delimiter */
    if (!match(TOKEN_DELIM)) {
        syntax_error("Expected closing delimiter ($$)");
        if (ast_root) free_ast(ast_root);
        return 1;
    }

    /* Check for syntax errors */
    if (syntax_error_occurred || !ast_root) {
        if (ast_root) free_ast(ast_root);
        return 1;
    }

    /* Transform the AST to CNF using our safe transform function */
    struct ast* cnf = safe_transform(ast_root);
    free_ast(ast_root); /* Free the original AST */

    if (!cnf) {
        fprintf(stderr, "Error transforming AST to CNF\n");
        printf("NO-SOLUTION\n");
        return 1;
    }

    CNF* flat_cnf = ast_to_cnf(cnf);

    if (!flat_cnf) {
        fprintf(stderr, "Error flattening CNF\n");
        free_ast(cnf);
        printf("NO-SOLUTION\n");
        return 1;
    }

    printf("\n");
    process_input(flat_cnf);

    /* Note: process_input frees flat_cnf, so we don't need to free it here */
    /* We need to free the CNF AST as it's a separate structure from flat_cnf */
    free_ast(cnf);

    return 0;
}

/* Entry point - replaces yyparse() */
int yyparse() {
    int result = parse_input();

    /* Make sure to free any remaining token */
    free_current_token();

    return result;
}
