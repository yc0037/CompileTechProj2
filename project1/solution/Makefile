LEX=flex
YACC=bison
CC=g++
OBJECT=main

#$(OBJECT): lex.yy.o  test.tab.o
#	$(CC) lex.yy.o test.tab.o -o $(OBJECT)
#	@./$(OBJECT)


#lex.yy.o: lex.yy.c parser.tab.h syntaxtree.hpp
#	$(CC) -c lex.yy.c

#test.tab.o: test.tab.c syntaxtree.hpp
#	$(CC) -c test.tab.c

#parser.tab.c  parser.tab.h: parser.y
#	$(YACC) -d parser.y

#lex.yy.c: parser.lex
#	$(LEX) parser.lex
all:
	$(YACC) -d parser.y
	$(LEX) parser.lex

clean:
	@rm -f $(OBJECT)  *.o lex.yy.cc parser.tab.cc parser.tab.h
