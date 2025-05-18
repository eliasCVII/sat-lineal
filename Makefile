lexer: sat-lex.l sat-parser.y sat-header.h sat-funcs.c
	bison -d sat-parser.y
	flex sat-lex.l
	gcc -o sat sat-parser.tab.c lex.yy.c sat-funcs.c

clean:
	rm -f sat-parser.tab.* lex.yy.c sat

test: sat
	./sat < expr
