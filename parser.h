#ifndef PARSER_H
#define PARSER_H

/**
 * parser.h - Parser interface
 */

#include "ast.h"
#include "cnf.h"

/* Token definitions - must match those in the lexer */
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
extern char* yylval_atomo;  /* Current token value */
extern void free_current_token();  /* Function to free the current token */

/* Parser functions */
int yyparse();  /* Main parsing function */
void yyerror(const char *msg);  /* Error reporting function */

/* Recursive descent parser functions */
struct ast* parse_exp();
struct ast* parse_implies_exp();
struct ast* parse_or_exp();
struct ast* parse_and_exp();
struct ast* parse_not_exp();

/* Process the parsed input */
void process_input(CNF* cnf);

#endif /* PARSER_H */
