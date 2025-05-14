/*-*-mode: c-*-*/

%{
#include "lexer.tab.h"
#include "lexerast.h"

char atomo[2]={0};
int yylex();
%}

%option noyywrap

%%

\$\$             { printf("DELIM "); return DELIM; }
\\rightarrow     { printf("IMPLIES "); return IMPLIES; }
\\vee            { printf("OR "); return OR; }
\\wedge          { printf("AND "); return AND; }
\\neg            { printf("NOT "); return NOT; }
"("              { printf("LPAREN "); return LPAREN;}
")"              { printf("RPAREN "); return RPAREN;}
[a-zA-Z0-9]      {
                printf("VAR(%s) ", yytext);
                yylval.atomo = strdup(yytext);
                if (!yylval.atomo) {
                        fprintf(stderr, "Memory allocation error\n");
                        YY_FATAL_ERROR("out of memory");
                }
                return VAR;
                }
[ \t\n]          ; /* ignore whitespace */
.                { fprintf(stderr, "ERROR: Invalid token '%s'\n", yytext);
}

%%
