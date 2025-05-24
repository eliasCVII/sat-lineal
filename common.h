#ifndef COMMON_H
#define COMMON_H

/*
 *
 * common.h: Utilidades que se usan en lex para manejar errores y liberar memoria
 *
 */

#include <stdio.h>

extern int yylex();
extern int yylex_destroy();

extern int syntax_error_occurred;
void yyerror(const char *msg);

static inline void syntax_error(const char *message) {
    yyerror(message);
}

#endif