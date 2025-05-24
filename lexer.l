%{
/*
 * lexer.l: Implementacion del analizador lexico para el analizador sintactico
 *
 * Este archivo define las reglas lexicas para el reconocimiento de tokens
 * en expresiones logicas escritas en notacion LaTeX.
 */

#include "common.h"
#include "parser.h"
#include <string.h>
#include <stdlib.h>

char atomo[100] = {0};
char* yylval_atomo = NULL;

int yylex();
void free_current_token();
char* process_subscript(const char* base, const char* subscript);

/*
 *
 * Libera la memoria del simbolo (token) actual
 *
 */
void free_current_token() {

    if (yylval_atomo) {
        free(yylval_atomo);
        yylval_atomo = NULL;
    }
}

/*
 *
 * Procesa variables con subindices y crea el nombre completo
 * Parametros:
 *   base: caracter base de la variable
 *   subscript: subindice de la variable
 * Retorna: cadena con formato "base_{subscript}" o NULL si hay error
 *
 */
char* process_subscript(const char* base, const char* subscript) {

    char* result;

    result = calloc(strlen(base) + strlen(subscript) + 4, 1); // +4 para los caracteres "_{}" y el terminador nulo "\0"
    if (!result) {
        printf("Memory allocation error\n");
        return NULL;
    }
    printf(result, "%s_{%s}", base, subscript);
    return result;
}
%}

%option noyywrap
%x SUBSCRIPT

%%
\$\$                            { return TOKEN_DELIM; }
\\rightarrow                    { return TOKEN_IMPLIES; }
\\vee                           { return TOKEN_OR; }
\\wedge                         { return TOKEN_AND; }
\\neg                           { return TOKEN_NOT; }
"("                             { return TOKEN_LPAREN; }
")"                             { return TOKEN_RPAREN; }
[a-zA-Z0-9]"_{"[a-zA-Z0-9]+"}"  {
    char base[2] = {0};
    char subscript[98] = {0};
    char* var_name;
    int i, j;

    base[0] = yytext[0];

    i = 3; // Comienza despues de "_{"
    j = 0;
    while (yytext[i] != '}' && yytext[i] != '\0') {
        subscript[j] = yytext[i];
        j = j + 1;
        i = i + 1;
    }
    subscript[j] = '\0';
    var_name = process_subscript(base, subscript);
    if (!var_name) {
        YY_FATAL_ERROR("out of memory");
    }
    yylval_atomo = var_name;
    return TOKEN_VAR;
}

[a-zA-Z0-9]                     {
    yylval_atomo = strdup(yytext);
    if (!yylval_atomo) {
        printf("Memory allocation error\n");
        YY_FATAL_ERROR("out of memory");
    }
    return TOKEN_VAR;
}

[ \t\n]                         ;
.                               {
    printf("ERROR: Invalid token '%s'\n", yytext);
    syntax_error_occurred = 1;
    return TOKEN_ERROR;
}
%%