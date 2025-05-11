#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "schema.h"
#include "csv.h"
#include "helper.h"

/* These are defined in parser.y */
extern int yyparse(void);
extern void yyerror(const char* s);
extern void reset_parser();
extern ASTNode* get_ast_root();

/* This is defined in scanner.l */
extern FILE* yyin;
extern void reset_scanner();

int main(int argc, char** argv) {
    /* Parse command line arguments */
    int should_printast = 0;
    char* output_dir = NULL;
    
    /* Handle help flag explicitly before parsing other args */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            return 0;
        }
    }

    parseargs(argc, argv, &should_printast, &output_dir);
    
    /* Read from stdin by default */
    yyin = stdin;
    
    /* Parse the input JSON */
    int parse_result = yyparse();
    if (parse_result != 0) {
        fprintf(stderr, "Error: JSON parsing failed\n");
        free(output_dir);
        return 1;
    }
    
    /* Get the AST root */
    ASTNode* ast = get_ast_root();
    if (!ast) {
        fprintf(stderr, "Error: Failed to build AST\n");
        free(output_dir);
        return 1;
    }
    
    /* Print AST if requested */
    if (should_printast) {
        printf("\n%s===== Standard AST =====%s\n", "\033[1;37m", "\033[0m");
        printast(ast, 0);
    }
    
    /* Generate schema from AST */
    Schema* schema = makeSchema();
    genSchema(schema, ast);
    
    /* Generate CSV files */
    makecsv(schema, ast, output_dir);
    
    /* Clean up */
    delSchema(schema);
    deleteast(ast);
    free(output_dir);
    
    return 0;
}
