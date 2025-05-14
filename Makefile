lexer: lexer.l lexer.y lexerast.h lexerfuncs.c
	bison -d lexer.y
	flex lexer.l
	gcc -o lexer lexer.tab.c lex.yy.c lexerfuncs.c

clean:
	rm -f lexer.tab.* lex.yy.c lexer

test: lexer
	./lexer < expr
