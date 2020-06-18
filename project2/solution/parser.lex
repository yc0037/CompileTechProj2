%{
#include "syntaxtree.hpp"
#define YYSTYPE Node*
#include "parser.tab.h"
extern Node* yylval;
extern int id;
extern "C" {
    int yywrap(void);
    int yylex(void);
}

%}
letter ([A-Za-z])
digital ([0-9])
integer ([1-9]({digital})*)|[0]
float (([1-9]({digital})*)|[0])[.]{digital}+
identifier ({letter}({letter}|{digital})*)
ignored ([ \f\r\t\v\n]+)
%%
{identifier} {
    yylval = new Identifier(id++, yytext);
    return IDENTIFIER;
}

{integer} {
    yylval = new Integer(id++, atoi(yytext));
    return INTEGER;
}

{float} {
    yylval = new Float(id++, yytext);
    return FLOAT;
}

{ignored} { }

"=" {
    return EQ;
}

"+" {
    yylval = new Operator(id++, ADD);
    return OPERATOR;
}

"-" {
    yylval = new Operator(id++, SUB);
    return OPERATOR;
}

"*" {
    yylval = new Operator(id++, MUL);
    return OPERATOR;
}

"/" {
    yylval = new Operator(id++, FDIV);
    return OPERATOR;
}

"%" {
    yylval = new Operator(id++, MOD);
    return OPERATOR;
}

"//" {
    yylval = new Operator(id++, IDIV);
    return OPERATOR;
}

"<" {
    return LANGLE;
}

">" {
    return RANGLE;
}

"[" {
    return LSQUARE;
}

"]" {
    return RSQUARE;
}

"(" {
    return LBRACKET;
}

")" {
    return RBRACKET;
}

"," {
    return COMMA;
}

";" {
    return SEMICOLON;
}

. {

}

%%
int yywrap()
{
return 1;
}