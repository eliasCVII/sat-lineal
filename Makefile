.PHONY: all clean test test-all test-single test-valgrind

all: sat

# Build the SAT solver
sat: lexer.l parser.c ast.c ast_transform.c cnf.c solver.c main.c common.h
	flex lexer.l
	gcc -o sat lex.yy.c parser.c ast.c ast_transform.c cnf.c solver.c main.c -lm

clean:
	rm -f lex.yy.c sat

# Run a single test
test: sat
	./sat < ../expresion.txt

# Run all tests
test-all: sat
	cd .. && ./run_tests.sh sat/sat

# Run tests with Valgrind to check for memory leaks
test-valgrind: sat
	valgrind --leak-check=full ./sat < ../expresion.txt
