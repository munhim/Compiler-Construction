#ifndef CSV_H
#define CSV_H

#include "ast.h"
#include "schema.h"
void makecsv(Schema* schema, ASTNode* ast, const char* output_dir);
char* esc(const char* s);
void writecsv(Schema* schema, int table_index, const char* output_dir);
void writeobj(Schema* schema, int table_index, ASTNode* obj, FILE* fp, long parent_id, int index, const char* parent_table, const char* output_dir);
void scalarcsv(Schema* schema, int table_index, ASTNode* array, 
                           FILE* fp, long parent_id);

#endif 
