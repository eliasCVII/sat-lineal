.PHONY: all clean test test-all test-valgrind

all: tarea1.exe

# Build the SAT solver (linear-time implementation)
tarea1.exe: lexer.l parser.c ast.c linear_solver.c main.c common.h
	flex lexer.l
	gcc -o tarea1.exe lex.yy.c parser.c ast.c linear_solver.c main.c -lm

clean:
	rm -f lex.yy.c tarea1.exe

# Run a single test
test: tarea1.exe
	@echo "Input expression:"
	@cat expresion.txt
	@echo ""
	@echo "Solver output:"
	@./tarea1.exe < expresion.txt

# Run all tests
test-all: tarea1.exe
	./run_tests.sh ./tarea1.exe --show-input

# Run tests with Valgrind to check for memory leaks
test-valgrind: tarea1.exe
	valgrind --leak-check=full ./tarea1.exe < expresion.txt
