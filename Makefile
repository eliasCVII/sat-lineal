.PHONY: all clean test test-all test-single test-valgrind

all: sat

# Original Bison/Yacc-based build
sat-bison: sat-lex.l sat-parser.y sat-header.h sat-funcs.c
	bison -d sat-parser.y
	flex sat-lex.l
	gcc -o sat sat-parser.tab.c lex.yy.c sat-funcs.c

# New pure C parser build
sat: sat-lex-modified.l sat-parser.c sat-header.h sat-funcs.c sat-transform.c
	flex sat-lex-modified.l
	gcc -o sat lex.yy.c sat-parser.c sat-funcs.c sat-transform.c

clean:
	rm -f sat-parser.tab.* lex.yy.c sat

# Run the original test
test: sat expr
	./sat < expr

# Run all tests
test-all: sat run_tests.sh
	chmod +x run_tests.sh
	./run_tests.sh

# Run a specific test (usage: make test-single TEST=test01_simple_sat)
test-single: sat
	@if [ -z "$(TEST)" ]; then \
		echo "Please specify a test with TEST=<test_name>"; \
		echo "Available tests:"; \
		ls tests/*.expr | sed 's/tests\///g' | sed 's/\.expr//g'; \
		exit 1; \
	fi
	@if [ ! -f "tests/$(TEST).expr" ]; then \
		echo "Test file tests/$(TEST).expr not found"; \
		exit 1; \
	fi
	./sat < tests/$(TEST).expr

# Run tests with Valgrind to check for memory leaks
test-valgrind: sat
	valgrind --leak-check=full ./sat < expr
