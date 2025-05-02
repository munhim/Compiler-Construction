#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "csv.h"
#include "util.h"

/* Quote and escape a string for CSV */
char* csv_escape(const char* s) {
    if (!s) return strdup("");
    
    /* Count characters that need escaping (", \n, etc.) */
    size_t len = strlen(s);
    size_t escaped_len = len + 2; /* +2 for surrounding quotes */
    
    for (size_t i = 0; i < len; i++) {
        if (s[i] == '"') escaped_len++; /* We'll double each quote */
    }
    
    char* result = malloc(escaped_len + 1);
    if (!result) return NULL;
    
    /* Start with opening quote */
    result[0] = '"';
    
    /* Copy and escape the string */
    size_t j = 1;
    for (size_t i = 0; i < len; i++) {
        if (s[i] == '"') {
            /* Double quotes to escape them */
            result[j++] = '"';
            result[j++] = '"';
        } else {
            result[j++] = s[i];
        }
    }
    
    /* Add closing quote and null terminator */
    result[j++] = '"';
    result[j] = '\0';
    
    return result;
}

/* Convert a node value to CSV string */
char* node_to_csv_value(ASTNode* node) {
    if (!node) return strdup("");
    
    switch (node->type) {
        case NODE_STRING: {
            return csv_escape(node->value.string_val);
        }
        case NODE_INTEGER: {
            char buf[32];
            sprintf(buf, "%ld", node->value.int_val);
            return strdup(buf);
        }
        case NODE_NUMBER: {
            char buf[32];
            sprintf(buf, "%g", node->value.num_val);
            return strdup(buf);
        }
        case NODE_BOOLEAN: {
            return strdup(node->value.bool_val ? "true" : "false");
        }
        case NODE_NULL: {
            return strdup("");
        }
        default:
            return strdup("");
    }
}

/* Write CSV header row */
void write_csv_header(Schema* schema, int table_index, FILE* fp) {
    Table* table = &schema->tables[table_index];
    
    for (int i = 0; i < table->column_count; i++) {
        fprintf(fp, "%s", table->columns[i].name);
        if (i < table->column_count - 1) {
            fprintf(fp, ",");
        }
    }
    fprintf(fp, "\n");
}

/* Write a CSV row for a scalar array element */
void write_scalar_array_csv(Schema* schema, int table_index, ASTNode* array, 
                           FILE* fp, long parent_id) {
    if (!is_array(array) || table_index < 0 || table_index >= schema->table_count) return;
    
    /* Table reference for future use */
    /* Table* table = &schema->tables[table_index]; */
    
    /* Process each array element */
    for (int i = 0; i < array->value.array.element_count; i++) {
        ASTNode* item = array->value.array.elements[i];
        
        /* Get a unique ID for this row */
        long row_id = get_next_id();
        
        /* Write the row */
        fprintf(fp, "%ld,", row_id);     /* Primary key */
        fprintf(fp, "%ld,", parent_id);  /* Foreign key to parent */
        fprintf(fp, "%d,", i);           /* Array index */
        
        /* Value depends on the item type */
        char* value = node_to_csv_value(item);
        fprintf(fp, "%s", value);
        free(value);
        
        fprintf(fp, "\n");
    }
}

/* Write a CSV row for an object */
void write_object_csv(Schema* schema, int table_index, ASTNode* obj, FILE* fp, 
                      long parent_id, int index, const char* parent_table) {
    const char* output_dir = "."; /* Default to current directory */
    if (!is_object(obj) || table_index < 0 || table_index >= schema->table_count) return;
    
    Table* table = &schema->tables[table_index];
    
    /* Get ID for this row */
    long row_id = obj->node_id;
    
    /* First column is always ID */
    fprintf(fp, "%ld", row_id);
    
    /* Write each column according to the schema */
    for (int i = 1; i < table->column_count; i++) {
        fprintf(fp, ",");
        
        Column* col = &table->columns[i];
        
        if (col->type == COL_FOREIGN_KEY && parent_id > 0 && 
            col->references && parent_table && 
            strcmp(col->references, parent_table) == 0) {
            /* This is a foreign key to our parent */
            fprintf(fp, "%ld", parent_id);
        }
        else if (col->type == COL_INDEX && index >= 0) {
            /* This is an array index */
            fprintf(fp, "%d", index);
        }
        else if (col->type == COL_FOREIGN_KEY) {
            /* Look for a matching object in this object's properties */
            int found = 0;
            
            /* Extract the field name from the column name (remove _id suffix) */
            char* field_name = strdup(col->name);
            char* underscore = strrchr(field_name, '_');
            if (underscore && strcmp(underscore, "_id") == 0) {
                *underscore = '\0';
                
                /* Find the field in the object */
                ASTNode* field = ast_object_get(obj, field_name);
                if (field && is_object(field)) {
                    fprintf(fp, "%ld", field->node_id);
                    found = 1;
                }
            }
            
            free(field_name);
            
            if (!found) {
                /* If we didn't find a matching object, write empty */
                fprintf(fp, "%s", "");
            }
        }
        else {
            /* Regular scalar column - find in object by column name */
            ASTNode* value = ast_object_get(obj, col->name);
            if (value && is_scalar(value)) {
                char* str_val = node_to_csv_value(value);
                fprintf(fp, "%s", str_val);
                free(str_val);
            } else {
                fprintf(fp, "%s", "");
            }
        }
    }
    
    fprintf(fp, "\n");
    
    /* Now process nested objects and arrays */
    for (int i = 0; i < obj->value.object.pair_count; i++) {
        KeyValuePair* pair = obj->value.object.pairs[i];
        ASTNode* value = pair->value;
        
        if (is_object(value)) {
            /* Find the table for this object */
            char* signature = object_shape_signature(value);
            int child_table_index = get_table_index_by_signature(schema, signature);
            free(signature);
            
            if (child_table_index >= 0) {
                /* Get the CSV file for this table */
                char path[512];
                sprintf(path, "%s/%s.csv", output_dir, schema->tables[child_table_index].name);
                
                FILE* child_fp = fopen(path, "a");
                if (child_fp) {
                    write_object_csv(schema, child_table_index, value, child_fp, 
                                     row_id, -1, table->name);
                    fclose(child_fp);
                }
            }
        }
        else if (is_array(value)) {
            if (value->value.array.element_count > 0) {
                ASTNode* first = value->value.array.elements[0];
                
                if (is_scalar(first)) {
                    /* Array of scalars - find junction table */
                    int junction_table_index = get_table_index(schema, pair->key);
                    if (junction_table_index >= 0) {
                        /* Get the CSV file for this table */
                        char path[512];
                        sprintf(path, "%s/%s.csv", output_dir, schema->tables[junction_table_index].name);
                        
                        FILE* junction_fp = fopen(path, "a");
                        if (junction_fp) {
                            write_scalar_array_csv(schema, junction_table_index, value, 
                                                   junction_fp, row_id);
                            fclose(junction_fp);
                        }
                    }
                }
                else if (is_object(first)) {
                    /* Array of objects - process each one */
                    for (int j = 0; j < value->value.array.element_count; j++) {
                        ASTNode* item = value->value.array.elements[j];
                        if (is_object(item)) {
                            /* Find the table for this object */
                            char* signature = object_shape_signature(item);
                            int child_table_index = get_table_index_by_signature(schema, signature);
                            free(signature);
                            
                            if (child_table_index >= 0) {
                                /* Get the CSV file for this table */
                                char path[512];
                                sprintf(path, "%s/%s.csv", output_dir, 
                                       schema->tables[child_table_index].name);
                                
                                FILE* child_fp = fopen(path, "a");
                                if (child_fp) {
                                    write_object_csv(schema, child_table_index, item, child_fp, 
                                                    row_id, j, table->name);
                                    fclose(child_fp);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

/* Write a single CSV file for a table */
void write_csv_file(Schema* schema, int table_index, const char* output_dir) {
    if (table_index < 0 || table_index >= schema->table_count) return;
    
    Table* table = &schema->tables[table_index];
    
    /* Create file path */
    char path[512];
    sprintf(path, "%s/%s.csv", output_dir, table->name);
    
    /* Create the file */
    FILE* fp = fopen(path, "w");
    if (!fp) {
        fprintf(stderr, "Error: Could not create CSV file %s: %s\n", 
                path, strerror(errno));
        return;
    }
    
    /* Write header row */
    write_csv_header(schema, table_index, fp);
    
    /* The data rows will be written by other functions */
    fclose(fp);
}

/* Generate CSV files from an AST using the schema */
void generate_csv_files(Schema* schema, ASTNode* ast, const char* output_dir) {
    if (!schema || !ast) return;
    
    /* Create output directory if it doesn't exist */
    struct stat st;
    if (stat(output_dir, &st) == -1) {
        if (mkdir(output_dir, 0755) != 0) {
            fprintf(stderr, "Error: Could not create output directory %s: %s\n", 
                    output_dir, strerror(errno));
            return;
        }
    }
    
    /* Create all the CSV files (with headers) */
    for (int i = 0; i < schema->table_count; i++) {
        write_csv_file(schema, i, output_dir);
    }
    
    /* Write CSV data for the root node */
    if (is_object(ast)) {
        /* Find which table the root belongs to */
        char* signature = object_shape_signature(ast);
        int root_table_index = get_table_index_by_signature(schema, signature);
        free(signature);
        
        if (root_table_index >= 0) {
            /* Append to the CSV file */
            char path[512];
            sprintf(path, "%s/%s.csv", output_dir, schema->tables[root_table_index].name);
            
            FILE* fp = fopen(path, "a");
            if (fp) {
                write_object_csv(schema, root_table_index, ast, fp, 0, -1, NULL);
                fclose(fp);
            }
        }
    }
    else if (is_array(ast)) {
        /* Handle array root */
        if (ast->value.array.element_count > 0) {
            ASTNode* first = ast->value.array.elements[0];
            
            if (is_object(first)) {
                /* Array of objects as root */
                for (int i = 0; i < ast->value.array.element_count; i++) {
                    ASTNode* item = ast->value.array.elements[i];
                    if (is_object(item)) {
                        char* signature = object_shape_signature(item);
                        int table_index = get_table_index_by_signature(schema, signature);
                        free(signature);
                        
                        if (table_index >= 0) {
                            char path[512];
                            sprintf(path, "%s/%s.csv", output_dir, schema->tables[table_index].name);
                            
                            FILE* fp = fopen(path, "a");
                            if (fp) {
                                write_object_csv(schema, table_index, item, fp, 0, i, "root");
                                fclose(fp);
                            }
                        }
                    }
                }
            }
            else if (is_scalar(first)) {
                /* Array of scalars as root */
                int table_index = get_table_index(schema, "values");
                if (table_index >= 0) {
                    char path[512];
                    sprintf(path, "%s/%s.csv", output_dir, schema->tables[table_index].name);
                    
                    FILE* fp = fopen(path, "a");
                    if (fp) {
                        write_scalar_array_csv(schema, table_index, ast, fp, 0);
                        fclose(fp);
                    }
                }
            }
        }
    }
}
