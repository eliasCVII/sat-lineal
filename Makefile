.PHONY: all clean test test-all test-single test-valgrind

all: tarea1.exe

# Build the SAT solver
tarea1.exe: lexer.l parser.c ast.c ast_transform.c cnf.c solver.c main.c common.h
	flex lexer.l
	gcc -o tarea1.exe lex.yy.c parser.c ast.c ast_transform.c cnf.c solver.c main.c -lm

clean:
	rm -f lex.yy.c tarea1.exe

# Run a single test
test: tarea1.exe
	./tarea1.exe < expresion.txt

# Run all tests
test-all: tarea1.exe
	cd .. && ./run_tests.sh sat/tarea1.exe

# Run tests with Valgrind to check for memory leaks
test-valgrind: tarea1.exe
	valgrind --leak-check=full ./tarea1.exe < expresion.txt
