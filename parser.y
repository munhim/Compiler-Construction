%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

extern int yylex(void);
extern FILE* yyin;
extern int yycolumn, yyline;

void yyerror(const char* s);

/* Root node of our AST */
ASTNode* ast_root = NULL;
%}

/* Bison declarations */
%define parse.error verbose
%locations

%union {
    long ival;
    double dval;
    char* sval;
    int bval;
    ASTNode* node;
    KeyValuePair* kvpair;
    KeyValuePair** kvpairs;
    ASTNode** nodes;
}

/* Terminal symbols */
%token <ival> INTEGER
%token <dval> NUMBER
%token <sval> STRING
%token <bval> BOOLEAN
%token NULLVAL

/* Define tokens for punctuation to avoid character value conflicts */
%token LBRACE RBRACE LBRACKET RBRACKET COLON COMMA

/* Non-terminal types */
%type <node> value object array
%type <kvpair> pair
%type <kvpairs> pairs
%type <nodes> values

/* Precedence (not really needed for JSON but good practice) */
%left LBRACE RBRACE
%left LBRACKET RBRACKET

%%

/* Grammar rules */
json:
    value           { ast_root = $1; }
    ;

object:
    '{' '}'         { $$ = objnode(NULL, 0); }
    | '{' pairs '}' { 
        int count = 0;
        while ($2[count] != NULL) count++;
        $$ = objnode($2, count);
    }
    ;

pairs:
    pair            { 
        $$ = calloc(2, sizeof(KeyValuePair*));
        if (!$$) {
            yyerror("Memory allocation failed");
            YYABORT;
        }
        $$[0] = $1;
        $$[1] = NULL;
    }
    | pairs ',' pair { 
        int count = 0;
        while ($1[count] != NULL) count++;
        $$ = realloc($1, (count + 2) * sizeof(KeyValuePair*));
        if (!$$) {
            yyerror("Memory allocation failed");
            YYABORT;
        }
        $$[count] = $3;
        $$[count + 1] = NULL;
    }
    ;

pair:
    STRING ':' value { 
        $$ = createKVpair($1, $3);
        free($1); /* Free the string as it's copied in createKVpair */
    }
    ;

array:
    '[' ']'         { $$ = arrnode(NULL, 0); }
    | '[' values ']' { 
        int count = 0;
        while ($2[count] != NULL) count++;
        $$ = arrnode($2, count);
    }
    ;

values:
    value           { 
        $$ = calloc(2, sizeof(ASTNode*));
        if (!$$) {
            yyerror("Memory allocation failed");
            YYABORT;
        }
        $$[0] = $1;
        $$[1] = NULL;
    }
    | values ',' value { 
        int count = 0;
        while ($1[count] != NULL) count++;
        $$ = realloc($1, (count + 2) * sizeof(ASTNode*));
        if (!$$) {
            yyerror("Memory allocation failed");
            YYABORT;
        }
        $$[count] = $3;
        $$[count + 1] = NULL;
    }
    ;

value:
    object          { $$ = $1; }
    | array         { $$ = $1; }
    | STRING        { $$ = strnode($1); free($1); }
    | INTEGER       { $$ = intnode($1); }
    | NUMBER        { $$ = numnode($1); }
    | BOOLEAN       { $$ = boolnode($1); }
    | NULLVAL       { $$ = nullnode(); }
    ;

%%

void yyerror(const char* s) {
    fprintf(stderr, "Error: %s at line %d, column %d\n", 
            s, yylloc.first_line, yylloc.first_column);
    fprintf(stderr, "DEBUG: Last token type: %d\n", yychar);
    
    /* Print token definitions for debugging */
    fprintf(stderr, "TOKEN VALUES - INTEGER: %d, NUMBER: %d, STRING: %d, BOOLEAN: %d, NULLVAL: %d\n",
            INTEGER, NUMBER, STRING, BOOLEAN, NULLVAL);
    fprintf(stderr, "PUNCT TOKENS - LBRACE: %d, RBRACE: %d, LBRACKET: %d, RBRACKET: %d, COLON: %d, COMMA: %d\n",
            LBRACE, RBRACE, LBRACKET, RBRACKET, COLON, COMMA);    
}

/* Get the AST root after parsing */
ASTNode* get_ast_root() {
    return ast_root;
}

/* Reset parser for multiple runs */
void reset_parser() {
    ast_root = NULL;
}
