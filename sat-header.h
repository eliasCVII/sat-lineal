#ifndef AST_H
#define AST_H

#define MAX_CLAUSES 100
#define MAX_LITERALS 100

extern int yylex();
extern int yyparse();
void yyerror(const char *msg);
int yylex_destroy();

typedef enum {
  NODE_VAR,
  NODE_NOT,
  NODE_AND,
  NODE_OR,
  NODE_IMPLIES,
  NODE_PAREN
} NodeType;

typedef struct ast {
  NodeType type;
  union {
    char *var_name;
    struct ast *child;
    struct {
      struct ast *left;
      struct ast *right;
    } binop;
  } data;
}ast;

typedef struct Literal {
  char* var;
  int negated;
} Literal;

typedef struct Clause {
  Literal* literals;
  int count;
} Clause;

typedef struct CNF {
  Clause* clauses;
  int count;
} CNF;

typedef struct Assignment {
  char** variables;
  int* values;
  int size;
} Assignment;

// Construye AST basado en la expresion
struct ast *make_var_node(char* name);
struct ast *make_unary_node(NodeType type, ast* child);
struct ast *make_binary_node(NodeType type, ast* l, ast* r);

// Traduce AST a CNF
struct ast* transform(struct ast* node);

// Assignment management
Assignment *create_assignment();
void assign_variable(Assignment *assn, char *var, int value);
int get_variable_value(Assignment *assn, char *var);
void free_assignment(Assignment *assn);

// Evaluation
int evaluate_clause(Clause *clause, Assignment *assn);
int evaluate_cnf(CNF *cnf, Assignment *assn);

// Propagation
int find_forced_assignments(CNF *cnf, Assignment *assn, Assignment *forced);

struct ast* demorgan(struct ast* node);
struct ast *distribute_OR(struct ast* left, struct ast* right);
struct ast* to_cnf(struct ast* node);

Literal* ast_to_literal(struct ast* node);
void flatten_or(struct ast* node, Clause* clause);
CNF* ast_to_cnf(struct ast* node);

// Solving SAT
void solve(CNF *cnf, Assignment *assn);

void process_input(CNF* cnf);

// Liberando memoria
void free_ast(ast* node);
void free_literal(Literal* lit);
void free_clause(Clause* clause);
void free_cnf(CNF* cnf);

// Print
void print_ast(ast* node);
void print_ast_latex(ast* node);
void print_cnf(CNF *cnf);

#endif
