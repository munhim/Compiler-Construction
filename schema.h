#ifndef SCHEMA_H
#define SCHEMA_H

#include "ast.h"

/* Maximum number of tables we can track */
#define MAX_TABLES 100

/* Maximum number of columns in a table */
#define MAX_COLUMNS 128

/* Column types */
typedef enum {
    COL_ID,         /* Primary key */
    COL_FOREIGN_KEY, /* Foreign key to parent table */
    COL_INDEX,      /* Array index */
    COL_STRING,     /* String value */
    COL_INTEGER,    /* Integer value */
    COL_NUMBER,     /* Floating point value */
    COL_BOOLEAN,    /* Boolean value */
} ColumnType;

/* Column definition */
typedef struct {
    char* name;         /* Column name */
    ColumnType type;    /* Column type */
    char* references;   /* For foreign keys, the table it references */
} Column;

/* Table schema */
typedef struct {
    char* name;                 /* Table name */
    Column columns[MAX_COLUMNS]; /* Column definitions */
    int column_count;           /* Number of columns */
    char* signature;            /* Object signature for this table (if from objects) */
    int is_junction;            /* True if this is a junction table (for array of scalars) */
    int is_child;               /* True if this is a child table (for array of objects) */
} Table;

/* Schema manager */
typedef struct {
    Table tables[MAX_TABLES];   /* All tables in the schema */
    int table_count;            /* Number of tables */
} Schema;

/* Schema functions */
Schema* create_schema();
void free_schema(Schema* schema);

/* Generate a schema from an AST */
void generate_schema(Schema* schema, ASTNode* ast);

/* Add a table to the schema */
int add_table(Schema* schema, const char* name, int is_junction, int is_child);

/* Add a column to a table */
void add_column(Schema* schema, int table_index, const char* name, ColumnType type, const char* references);

/* Check if a table with the given name already exists */
int table_exists(Schema* schema, const char* name);

/* Check if a table with the given signature already exists */
int table_with_signature_exists(Schema* schema, const char* signature);

/* Get the index of a table by name */
int get_table_index(Schema* schema, const char* name);

/* Get the index of a table by signature */
int get_table_index_by_signature(Schema* schema, const char* signature);

/* Print the schema */
void print_schema(Schema* schema);

/* Process an object node and add it to the appropriate table */
void process_object(Schema* schema, ASTNode* obj, const char* parent_table, long parent_id, int array_index);

#endif /* SCHEMA_H */
