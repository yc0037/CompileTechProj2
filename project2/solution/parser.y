%{
#include "syntaxtree.hpp"

extern "C" {
    extern int yylex(void);
    void yyerror(const char *s);
}

#define YYSTYPE Node*

using namespace std;
using namespace Boost::Internal;

int id = 0;
Function* mainFunc = NULL;
%}
%token IDENTIFIER INTEGER FLOAT  OPERATOR
%token EQ LANGLE RANGLE LSQUARE RSQUARE LBRACKET RBRACKET COMMA SEMICOLON
%%
program : stmt {
                $$ = mainFunc;
                ((Function *) $$) -> insertStmt($1);
        }
        |    stmt program {
                $$ = $2;
                ((Function *) $$) -> insertStmt($1);
        }
stmt : tref EQ rhs SEMICOLON {
                $$ = new MoveStmt(id++, $1, $3);
        };
rhs : rhs OPERATOR rhs {
                $$ = new RHS(id++, $1, ((Operator *)$2) -> op, $3);
                if ($2 != NULL) delete $2;
        }
        |   LBRACKET rhs RBRACKET {
                $$ = new RHS(id++, $2, BRACKET, NULL);
        }
        |   tref {
                $$ = new RHS(id++, $1, NONE, NULL);
        }
        |   sref {
                $$ = new RHS(id++, $1, NONE, NULL);
        }
        |   INTEGER {
                $$ = new RHS(id++, $1, NONE, NULL);
        }
        |   FLOAT {
                $$ = new RHS(id++, $1, NONE, NULL);
        };
idexpr : IDENTIFIER {
        $$ = new IdExpr(id++, $1, NONE, NULL);
        }
        |   idexpr OPERATOR idexpr {
                $$ = new IdExpr(id++, $1, ((Operator *)$2) -> op, $3);
                if ($2 != NULL) delete $2;
        }
        |   idexpr OPERATOR INTEGER {
                $$ = new IdExpr(id++, $1, ((Operator *)$2) -> op, $3);
                if ($2 != NULL) delete $2;
        }
        |   LBRACKET idexpr RBRACKET {
                $$ = new IdExpr(id++, $2, BRACKET, NULL);
        };
clist : INTEGER {
                $$ = new CList(id++);
                if ($1 != NULL) {
                        ((CList *) $$) -> insertNum(((Integer *)$1) -> value);
                        delete $1;
                }
        }
        |    INTEGER COMMA clist {
                $$ = $3;
                if ($1 != NULL) {
                        ((CList *) $$) -> insertNum(((Integer *)$1) -> value);
                        delete $1;
                }
        };
alist : idexpr {
                $$ = new AList(id++);
                ((AList *) $$) -> insertNode($1);
        }
        |    idexpr COMMA alist {
                $$ = $3;
                ((AList *) $$) -> insertNode($1);
        };
tref : IDENTIFIER LANGLE clist RANGLE LSQUARE alist RSQUARE {
        $$ = new Tref(id++, ((Identifier *)$1) -> name);
        delete $1;
        ((Tref *) $$) -> ranges = ((CList *) $3) -> numlist;
        delete $3;
        ((Tref *) $$) -> idExprs = ((AList *) $6) -> nodelist;
        delete $6;
};
sref : IDENTIFIER LANGLE INTEGER RANGLE {
        $$ = new Sref(id++, ((Identifier *)$1) -> name);
        delete $1;
};
%%
void yyerror(const char* s)
{
printf("Error encountered: %s \n",  s);
}