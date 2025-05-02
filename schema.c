#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "schema.h"

/* Create a new schema */
Schema* create_schema() {
    Schema* schema = calloc(1, sizeof(Schema));
    return schema;
}

/* Free schema resources */
void free_schema(Schema* schema) {
    if (!schema) return;
    
    for (int i = 0; i < schema->table_count; i++) {
        if (schema->tables[i].name) {
            free(schema->tables[i].name);
        }
        if (schema->tables[i].signature) {
            free(schema->tables[i].signature);
        }
        
        for (int j = 0; j < schema->tables[i].column_count; j++) {
            if (schema->tables[i].columns[j].name) {
                free(schema->tables[i].columns[j].name);
            }
            if (schema->tables[i].columns[j].references) {
                free(schema->tables[i].columns[j].references);
            }
        }
    }
    
    free(schema);
}

/* Add a table to the schema */
int add_table(Schema* schema, const char* name, int is_junction, int is_child) {
    if (schema->table_count >= MAX_TABLES) {
        fprintf(stderr, "Maximum number of tables exceeded\n");
        return -1;
    }
    
    int table_index = schema->table_count++;
    schema->tables[table_index].name = strdup(name);
    schema->tables[table_index].column_count = 0;
    schema->tables[table_index].signature = NULL;
    schema->tables[table_index].is_junction = is_junction;
    schema->tables[table_index].is_child = is_child;
    
    /* Every table needs an ID column */
    add_column(schema, table_index, "id", COL_ID, NULL);
    
    return table_index;
}

/* Add a column to a table */
void add_column(Schema* schema, int table_index, const char* name, ColumnType type, const char* references) {
    if (table_index < 0 || table_index >= schema->table_count) return;
    
    Table* table = &schema->tables[table_index];
    if (table->column_count >= MAX_COLUMNS) {
        fprintf(stderr, "Maximum number of columns exceeded for table %s\n", table->name);
        return;
    }
    
    Column* col = &table->columns[table->column_count++];
    col->name = strdup(name);
    col->type = type;
    col->references = references ? strdup(references) : NULL;
}

/* Check if a table with the given name already exists */
int table_exists(Schema* schema, const char* name) {
    for (int i = 0; i < schema->table_count; i++) {
        if (strcmp(schema->tables[i].name, name) == 0) {
            return 1;
        }
    }
    return 0;
}

/* Check if a table with the given signature already exists */
int table_with_signature_exists(Schema* schema, const char* signature) {
    if (!signature) return 0;
    
    for (int i = 0; i < schema->table_count; i++) {
        if (schema->tables[i].signature && 
            strcmp(schema->tables[i].signature, signature) == 0) {
            return 1;
        }
    }
    return 0;
}

/* Get the index of a table by name */
int get_table_index(Schema* schema, const char* name) {
    for (int i = 0; i < schema->table_count; i++) {
        if (strcmp(schema->tables[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

/* Get the index of a table by signature */
int get_table_index_by_signature(Schema* schema, const char* signature) {
    if (!signature) return -1;
    
    for (int i = 0; i < schema->table_count; i++) {
        if (schema->tables[i].signature && 
            strcmp(schema->tables[i].signature, signature) == 0) {
            return i;
        }
    }
    return -1;
}

/* Guess a good table name from object properties or context */
char* guess_table_name(ASTNode* obj, const char* parent_key, const char* default_name) {
    /* Try to derive name from commonly used ID fields */
    const char* id_fields[] = {"type", "kind", "name", "category", "class", NULL};
    
    if (is_object(obj)) {
        for (int i = 0; id_fields[i] != NULL; i++) {
            ASTNode* id_node = ast_object_get(obj, id_fields[i]);
            if (id_node && id_node->type == NODE_STRING) {
                /* Convert to lowercase and remove non-alphanumeric chars */
                char* name = strdup(id_node->value.string_val);
                for (int j = 0; name[j]; j++) {
                    name[j] = tolower(name[j]);
                    if (!isalnum(name[j]) && name[j] != '_') {
                        name[j] = '_';
                    }
                }
                /* Add 's' to make it plural if it's not already */
                if (name[0] && name[strlen(name)-1] != 's') {
                    char* plural = malloc(strlen(name) + 2);
                    sprintf(plural, "%ss", name);
                    free(name);
                    return plural;
                }
                return name;
            }
        }
    }
    
    /* Use parent key as name if available (a common pattern) */
    if (parent_key) {
        /* Make it plural */
        char* name = strdup(parent_key);
        /* Convert to lowercase */
        for (int i = 0; name[i]; i++) {
            name[i] = tolower(name[i]);
            if (!isalnum(name[i]) && name[i] != '_') {
                name[i] = '_';
            }
        }
        return name;
    }
    
    /* Fall back to the default */
    return strdup(default_name);
}

/* Process scalar arrays (R3) */
void process_scalar_array(Schema* schema, ASTNode* array, const char* parent_table, 
                           long parent_id, const char* parent_key) {
    if (!is_array(array) || !parent_table || !parent_key) return;
    
    /* Create a junction table for this array */
    char table_name[256];
    sprintf(table_name, "%s", parent_key);
    
    int table_index;
    if (!table_exists(schema, table_name)) {
        table_index = add_table(schema, table_name, 1, 0);
        
        /* Add junction table columns */
        char fk_name[256];
        sprintf(fk_name, "%s_id", parent_table);
        add_column(schema, table_index, fk_name, COL_FOREIGN_KEY, parent_table);
        add_column(schema, table_index, "index", COL_INDEX, NULL);
        add_column(schema, table_index, "value", COL_STRING, NULL);
    } else {
        table_index = get_table_index(schema, table_name);
    }
}

/* Process an object node to generate schema */
void process_object(Schema* schema, ASTNode* obj, const char* parent_table, 
                    long parent_id, int array_index) {
    if (!is_object(obj)) return;
    
    /* Generate a signature for this object to identify its "shape" */
    char* signature = object_shape_signature(obj);
    
    /* Check if we already have a table for this signature */
    int table_index = -1;
    char* table_name = NULL;
    
    if (signature) {
        table_index = get_table_index_by_signature(schema, signature);
    }
    
    /* If no existing table, create one */
    if (table_index == -1) {
        /* Guess table name from object or context */
        if (!parent_table) {
            /* Root object gets the default name "root" */
            table_name = strdup("root");
        } else {
            /* For child objects, try to derive name from parent key or object properties */
            const char* parent_key = NULL;
            
            /* Find this object's key in its parent (if it has one) */
            if (obj->parent && obj->parent->type == NODE_OBJECT) {
                for (int i = 0; i < obj->parent->value.object.pair_count; i++) {
                    if (obj->parent->value.object.pairs[i]->value == obj) {
                        parent_key = obj->parent->value.object.pairs[i]->key;
                        break;
                    }
                }
            }
            
            /* Try to guess a good name */
            if (parent_key) {
                table_name = guess_table_name(obj, parent_key, parent_table);
            } else {
                table_name = strdup(parent_table);
            }
        }
        
        /* If we're in an array, this is a child table */
        int is_child = (array_index >= 0);
        
        /* Create the table */
        table_index = add_table(schema, table_name, 0, is_child);
        free(table_name);
        
        /* Store signature to identify objects with same structure */
        schema->tables[table_index].signature = signature;
        
        /* If this is a child object in an array, add parent foreign key and index */
        if (is_child && parent_table) {
            char fk_name[256];
            sprintf(fk_name, "%s_id", parent_table);
            add_column(schema, table_index, fk_name, COL_FOREIGN_KEY, parent_table);
            add_column(schema, table_index, "seq", COL_INDEX, NULL);
        }
        
        /* Add columns for each property in the object */
        for (int i = 0; i < obj->value.object.pair_count; i++) {
            KeyValuePair* pair = obj->value.object.pairs[i];
            ASTNode* value = pair->value;
            
            if (is_scalar(value)) {
                /* Simple column for scalar values (R4) */
                ColumnType col_type;
                switch (value->type) {
                    case NODE_STRING: col_type = COL_STRING; break;
                    case NODE_INTEGER: col_type = COL_INTEGER; break;
                    case NODE_NUMBER: col_type = COL_NUMBER; break;
                    case NODE_BOOLEAN: col_type = COL_BOOLEAN; break;
                    default: col_type = COL_STRING; break;
                }
                add_column(schema, table_index, pair->key, col_type, NULL);
            } 
            else if (is_object(value)) {
                /* Nested object becomes a foreign key (R1, R5) */
                process_object(schema, value, schema->tables[table_index].name, 0, -1);
                
                /* Add foreign key in this table */
                char fk_name[256];
                sprintf(fk_name, "%s_id", pair->key);
                add_column(schema, table_index, fk_name, COL_FOREIGN_KEY, pair->key);
            } 
            else if (is_array(value)) {
                /* Check what's in the array */
                if (value->value.array.element_count > 0) {
                    ASTNode* first = value->value.array.elements[0];
                    
                    if (is_scalar(first)) {
                        /* Array of scalars (R3) */
                        process_scalar_array(schema, value, schema->tables[table_index].name, 
                                             obj->node_id, pair->key);
                    } 
                    else if (is_object(first)) {
                        /* Array of objects (R2) */
                        /* Process each object in the array (they'll be grouped by signature) */
                        for (int j = 0; j < value->value.array.element_count; j++) {
                            ASTNode* item = value->value.array.elements[j];
                            if (is_object(item)) {
                                process_object(schema, item, schema->tables[table_index].name, 
                                              obj->node_id, j);
                            }
                        }
                    }
                }
            }
        }
    } else {
        /* Using existing table */
        free(signature); /* Don't need the signature since we found a matching table */
    }
}

/* Generate a schema from an AST */
void generate_schema(Schema* schema, ASTNode* ast) {
    if (!schema || !ast) return;
    
    /* Start with the root object */
    if (is_object(ast)) {
        process_object(schema, ast, NULL, 0, -1);
    } 
    else if (is_array(ast)) {
        /* Root is array - check first element to determine type */
        if (ast->value.array.element_count > 0) {
            ASTNode* first = ast->value.array.elements[0];
            
            if (is_object(first)) {
                /* Array of objects as root */
                for (int i = 0; i < ast->value.array.element_count; i++) {
                    ASTNode* item = ast->value.array.elements[i];
                    if (is_object(item)) {
                        process_object(schema, item, "root", 0, i);
                    }
                }
            } else {
                /* Array of scalars as root - unusual but possible */
                process_scalar_array(schema, ast, "root", 0, "values");
            }
        }
    }
}

/* Print schema (for debugging) */
void print_schema(Schema* schema) {
    if (!schema) return;
    
    printf("Schema (%d tables):\n", schema->table_count);
    for (int i = 0; i < schema->table_count; i++) {
        Table* table = &schema->tables[i];
        printf("Table: %s", table->name);
        if (table->is_junction) printf(" (junction)");
        if (table->is_child) printf(" (child)");
        printf("\n");
        
        for (int j = 0; j < table->column_count; j++) {
            Column* col = &table->columns[j];
            printf("  %s: ", col->name);
            
            switch (col->type) {
                case COL_ID: printf("ID"); break;
                case COL_FOREIGN_KEY: 
                    printf("FK -> %s", col->references ? col->references : "unknown"); 
                    break;
                case COL_INDEX: printf("INDEX"); break;
                case COL_STRING: printf("STRING"); break;
                case COL_INTEGER: printf("INTEGER"); break;
                case COL_NUMBER: printf("NUMBER"); break;
                case COL_BOOLEAN: printf("BOOLEAN"); break;
                default: printf("UNKNOWN"); break;
            }
            printf("\n");
        }
    }
}
