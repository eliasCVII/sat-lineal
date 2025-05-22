/*-*-mode: c-*-*/

%{
#include "sat-parser.tab.h"
#include "sat-header.h"
#include <string.h>
#include <stdlib.h>

char atomo[100]={0}; /* Increased buffer size to handle subscripts */
int yylex();

/* External declaration for the syntax error flag */
extern int syntax_error_occurred;

/* Helper function to process subscripted variables */
char* process_subscript(const char* base, const char* subscript) {
    char* result = malloc(strlen(base) + strlen(subscript) + 4); /* +4 for "_{}" and null terminator */
    if (!result) {
        fprintf(stderr, "Memory allocation error\n");
        return NULL;
    }
    sprintf(result, "%s_{%s}", base, subscript);
    return result;
}
%}

%option noyywrap

/* Define states for parsing subscripts */
%x SUBSCRIPT

%%

\$\$             { printf("DELIM "); return DELIM; }
\\rightarrow     { printf("IMPLIES "); return IMPLIES; }
\\vee            { printf("OR "); return OR; }
\\wedge          { printf("AND "); return AND; }
\\neg            { printf("NOT "); return NOT; }
"("              { printf("LPAREN "); return LPAREN;}
")"              { printf("RPAREN "); return RPAREN;}

 /* Match a variable followed by a subscript */
[a-zA-Z0-9]"_{"[a-zA-Z0-9]+"}" {
    /* Extract the base variable and subscript */
    char base[2] = {0};
    char subscript[98] = {0}; /* Large enough for any reasonable subscript */

    /* Extract the base character */
    base[0] = yytext[0];

    /* Extract the subscript (everything between { and }) */
    int i = 3; /* Start after "_{" */
    int j = 0;
    while (yytext[i] != '}' && yytext[i] != '\0') {
        subscript[j++] = yytext[i++];
    }
    subscript[j] = '\0';

    /* Create the full variable name */
    char* var_name = process_subscript(base, subscript);
    if (!var_name) {
        YY_FATAL_ERROR("out of memory");
    }

    printf("VAR(%s) ", var_name);
    yylval.atomo = var_name; /* No need to strdup, process_subscript already allocates memory */
    return VAR;
}

 /* Match a simple variable (for backward compatibility) */
[a-zA-Z0-9]      {
                  printf("VAR(%s) ", yytext);
                  yylval.atomo = strdup(yytext);
                  if (!yylval.atomo) {
                          fprintf(stderr, "Memory allocation error\n");
                          YY_FATAL_ERROR("out of memory");
                  }
                  return VAR;
                }

 /* Ignore whitespace */
[ \t\n]          ;

 /* Error for any other character */
.                {
                  fprintf(stderr, "ERROR: Invalid token '%s'\n", yytext);
                  syntax_error_occurred = 1;
                  return 0;  /* Return 0 to signal an error */
                }

%%
