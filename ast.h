#ifndef AST_H
#define AST_H

#include <stdio.h>

/* AST node types */
typedef enum {
    NODE_OBJECT,
    NODE_ARRAY,
    NODE_STRING,
    NODE_INTEGER,
    NODE_NUMBER,
    NODE_BOOLEAN,
    NODE_NULL
} NodeType;

/* Forward declaration */
typedef struct ASTNode ASTNode;

/* Key-value pair for objects */
typedef struct KeyValuePair {
    char* key;
    ASTNode* value;
} KeyValuePair;

/* AST node structure */
struct ASTNode {
    NodeType type;
    union {
        /* Object fields */
        struct {
            KeyValuePair** pairs;
            int pair_count;
        } object;
        
        /* Array fields */
        struct {
            ASTNode** elements;
            int element_count;
        } array;
        
        /* Scalar values */
        char* string_val;
        long int_val;
        double num_val;
        int bool_val;
    } value;

    /* Track the parent node for building relationships */
    ASTNode* parent;
    
    /* Unique ID for this node (used for primary/foreign keys) */
    long node_id;
};

/* AST node creation functions */
ASTNode* create_object_node(KeyValuePair** pairs, int count);
ASTNode* create_array_node(ASTNode** elements, int count);
ASTNode* create_string_node(char* value);
ASTNode* create_integer_node(long value);
ASTNode* create_number_node(double value);
ASTNode* create_boolean_node(int value);
ASTNode* create_null_node();

/* Key-value pair creation */
KeyValuePair* create_key_value_pair(char* key, ASTNode* value);

/* AST utility functions */
void print_ast(ASTNode* node, int indent);
void free_ast(ASTNode* node);

/* Find an object's key by name */
ASTNode* ast_object_get(ASTNode* obj, const char* key);

/* Compute a "shape signature" to detect objects with the same structure */
char* object_shape_signature(ASTNode* obj);

/* Get unique ID for this node */
long get_node_id(ASTNode* node);

/* Get the next auto-incrementing ID */
long get_next_id();

/* Check if object matches a given signature */
int object_matches_signature(ASTNode* obj, const char* signature);

/* Helper to check node types */
int is_object(ASTNode* node);
int is_array(ASTNode* node);
int is_scalar(ASTNode* node);

#endif /* AST_H */
