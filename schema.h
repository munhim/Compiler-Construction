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

Schema* makeSchema();
void delSchema(Schema* schema);
void genSchema(Schema* schema, ASTNode* ast);
int addT(Schema* schema, const char* name, int is_junction, int is_child);
void addC(Schema* schema, int table_index, const char* name, ColumnType type, const char* references);
int exists(Schema* schema, const char* name);
int colexist(Schema* schema, int table_index, const char* name);
int tableexists(Schema* schema, const char* signature);
int gettablei(Schema* schema, const char* name);
int getibysig(Schema* schema, const char* signature);
void printschema(Schema* schema);
void processobj(Schema* schema, ASTNode* obj, const char* parent_table, long parent_id, int array_index);

#endif 
