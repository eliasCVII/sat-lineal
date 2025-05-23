#ifndef COMMON_H
#define COMMON_H

/**
 * common.h - Common utilities, error handling
 */

#include <stdio.h>

/* External declarations for lexer functions */
extern int yylex();
extern int yylex_destroy();

/* Error handling */
extern int syntax_error_occurred;
void yyerror(const char *msg);

/* Utility function to report syntax errors */
static inline void syntax_error(const char *message) {
    yyerror(message);
}

#endif /* COMMON_H */
