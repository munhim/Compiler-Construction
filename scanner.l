%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "parser.tab.h"

/* Debugging function to print token names */
const char* token_name(int token) {
    switch(token) {
        case LBRACE: return "LBRACE";
        case RBRACE: return "RBRACE";
        case LBRACKET: return "LBRACKET";
        case RBRACKET: return "RBRACKET";
        case COLON: return "COLON";
        case COMMA: return "COMMA";
        case STRING: return "STRING";
        case INTEGER: return "INTEGER";
        case NUMBER: return "NUMBER";
        case BOOLEAN: return "BOOLEAN";
        case NULLVAL: return "NULLVAL";
        default: return "UNKNOWN";
    }
}

/* Track line and column for error reporting */
int yycolumn = 1;
int yyline = 1;

/* Update location tracking information */
#define YY_USER_ACTION { \
    yylloc.first_line = yylloc.last_line = yyline; \
    yylloc.first_column = yycolumn; \
    yylloc.last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng; \
}

/* Function to handle escaped characters in strings */
char* process_escapes(char* text, int len) {
    char* result = malloc(len + 1);
    if (!result) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    int i, j = 0;
    for (i = 0; i < len; i++) {
        if (text[i] == '\\' && i + 1 < len) {
            i++; // Skip backslash
            switch (text[i]) {
                case 'n': result[j++] = '\n'; break;
                case 'r': result[j++] = '\r'; break;
                case 't': result[j++] = '\t'; break;
                case 'b': result[j++] = '\b'; break;
                case 'f': result[j++] = '\f'; break;
                case '"': result[j++] = '"'; break;
                case '\\': result[j++] = '\\'; break;
                case '/': result[j++] = '/'; break;
                case 'u': {
                    // Handle Unicode escapes \uXXXX
                    if (i + 4 < len) {
                        char hex[5] = {text[i+1], text[i+2], text[i+3], text[i+4], 0};
                        int unicode_val;
                        sscanf(hex, "%x", &unicode_val);
                        
                        // Basic UTF-8 encoding for BMP characters
                        if (unicode_val <= 0x7F) {
                            result[j++] = (char)unicode_val;
                        } else if (unicode_val <= 0x7FF) {
                            result[j++] = (char)(0xC0 | (unicode_val >> 6));
                            result[j++] = (char)(0x80 | (unicode_val & 0x3F));
                        } else {
                            result[j++] = (char)(0xE0 | (unicode_val >> 12));
                            result[j++] = (char)(0x80 | ((unicode_val >> 6) & 0x3F));
                            result[j++] = (char)(0x80 | (unicode_val & 0x3F));
                        }
                        i += 4; // Skip the 4 hex digits
                    }
                    break;
                }
                default: result[j++] = text[i]; break;
            }
        } else {
            result[j++] = text[i];
        }
    }
    result[j] = '\0';
    return result;
}
%}

/* Flex options */
%option noinput
%option nounput

/* Exclusive state for string parsing */
%x STRING

%%

[ \t]+         { /* Ignore whitespace but count columns */ }
\n             { yyline++; yycolumn = 1; }
\r             { /* Ignore carriage return */ }

"{"            { fprintf(stderr, "DEBUG: Found { at line %d, column %d\n", yyline, yycolumn); fprintf(stderr, "DEBUG: Returning LBRACE token (value=%d)\n", LBRACE); return LBRACE; }
"}"            { fprintf(stderr, "DEBUG: Found } at line %d, column %d\n", yyline, yycolumn); fprintf(stderr, "DEBUG: Returning RBRACE token (value=%d)\n", RBRACE); return RBRACE; }
"["            { fprintf(stderr, "DEBUG: Found [ at line %d, column %d\n", yyline, yycolumn); fprintf(stderr, "DEBUG: Returning LBRACKET token (value=%d)\n", LBRACKET); return LBRACKET; }
"]"            { fprintf(stderr, "DEBUG: Found ] at line %d, column %d\n", yyline, yycolumn); fprintf(stderr, "DEBUG: Returning RBRACKET token (value=%d)\n", RBRACKET); return RBRACKET; }
":"            { fprintf(stderr, "DEBUG: Found : at line %d, column %d\n", yyline, yycolumn); fprintf(stderr, "DEBUG: Returning COLON token (value=%d)\n", COLON); return COLON; }
","            { fprintf(stderr, "DEBUG: Found , at line %d, column %d\n", yyline, yycolumn); fprintf(stderr, "DEBUG: Returning COMMA token (value=%d)\n", COMMA); return COMMA; }

"true"         { yylval.bval = 1; return BOOLEAN; }
"false"        { yylval.bval = 0; return BOOLEAN; }
"null"         { return NULLVAL; }

-?[0-9]+\.[0-9]+([eE][-+]?[0-9]+)? { 
    yylval.dval = atof(yytext); 
    fprintf(stderr, "DEBUG: Found float number %f at line %d, column %d\n", yylval.dval, yyline, yycolumn);
    return NUMBER; 
}

-?[0-9]+([eE][-+]?[0-9]+)? { 
    if (strchr(yytext, 'e') || strchr(yytext, 'E')) {
        yylval.dval = atof(yytext);
        fprintf(stderr, "DEBUG: Found scientific number %f at line %d, column %d\n", yylval.dval, yyline, yycolumn);
        return NUMBER;
    } else {
        yylval.ival = atol(yytext); 
        fprintf(stderr, "DEBUG: Found integer %ld at line %d, column %d\n", yylval.ival, yyline, yycolumn);
        return INTEGER;
    }
}

\"            { 
    fprintf(stderr, "DEBUG: Starting string at line %d, column %d\n", yyline, yycolumn);
    BEGIN(STRING); 
    yylval.sval = calloc(1, 1); /* Start with empty string */
    if (!yylval.sval) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
}

<STRING>[^\"\\]* {
    /* Append normal characters to string */
    fprintf(stderr, "DEBUG: Adding to string: '%s' at line %d, column %d\n", yytext, yyline, yycolumn);
    size_t len = strlen(yylval.sval);
    yylval.sval = realloc(yylval.sval, len + strlen(yytext) + 1);
    if (!yylval.sval) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    strcat(yylval.sval, yytext);
}

<STRING>\\. {
    /* Handle escaped characters */
    char* processed = process_escapes(yytext, yyleng);
    size_t len = strlen(yylval.sval);
    yylval.sval = realloc(yylval.sval, len + strlen(processed) + 1);
    if (!yylval.sval) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    strcat(yylval.sval, processed);
    free(processed);
}

<STRING>\" {
    fprintf(stderr, "DEBUG: Ending string with value '%s' at line %d, column %d\n", yylval.sval, yyline, yycolumn);
    BEGIN(INITIAL);
    fprintf(stderr, "DEBUG: Returning STRING token (value=%d)\n", STRING);
    return STRING;
}

<STRING><<EOF>> {
    fprintf(stderr, "Error: Unterminated string at line %d, column %d\n", 
            yyline, yycolumn);
    exit(1);
}

.              { 
    fprintf(stderr, "Error: Unexpected character '%c' at line %d, column %d\n", 
            yytext[0], yyline, yycolumn); 
    exit(1);
}

%%

/* Required when not using -lfl */
int yywrap() {
    return 1; /* Signal the end of input */
}

/* Reset scanner for multiple runs */
void reset_scanner() {
    yycolumn = 1;
    yyline = 1;
    BEGIN(INITIAL);
}
