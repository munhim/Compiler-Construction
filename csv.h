#ifndef CSV_H
#define CSV_H

#include "ast.h"
#include "schema.h"

/* Generate CSV files from an AST using the schema */
void generate_csv_files(Schema* schema, ASTNode* ast, const char* output_dir);

/* Quote and escape a string for CSV */
char* csv_escape(const char* s);

/* Write a single CSV file for a table */
void write_csv_file(Schema* schema, int table_index, const char* output_dir);

/* Write a CSV row for an object */
void write_object_csv(Schema* schema, int table_index, ASTNode* obj, FILE* fp, 
                      long parent_id, int index, const char* parent_table, const char* output_dir);

/* Write a scalar array junction table */
void write_scalar_array_csv(Schema* schema, int table_index, ASTNode* array, 
                           FILE* fp, long parent_id);

#endif /* CSV_H */
