#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

/* Global counter for node IDs */
static long next_node_id = 1;

/* Get the next unique ID */
long get_next_id() {
    return next_node_id++;
}

/* Reset the node ID counter (for testing) */
void reset_node_ids() {
    next_node_id = 1;
}

/* AST node creation functions */
ASTNode* create_object_node(KeyValuePair** pairs, int count) {
    ASTNode* node = calloc(1, sizeof(ASTNode));
    if (!node) return NULL;
    
    node->type = NODE_OBJECT;
    node->value.object.pairs = pairs;
    node->value.object.pair_count = count;
    node->parent = NULL;
    node->node_id = get_next_id();
    
    /* Set parent pointers for all children */
    for (int i = 0; i < count; i++) {
        if (pairs && pairs[i] && pairs[i]->value) {
            pairs[i]->value->parent = node;
        }
    }
    
    return node;
}

ASTNode* create_array_node(ASTNode** elements, int count) {
    ASTNode* node = calloc(1, sizeof(ASTNode));
    if (!node) return NULL;
    
    node->type = NODE_ARRAY;
    node->value.array.elements = elements;
    node->value.array.element_count = count;
    node->parent = NULL;
    node->node_id = get_next_id();
    
    /* Set parent pointers for all elements */
    for (int i = 0; i < count; i++) {
        if (elements && elements[i]) {
            elements[i]->parent = node;
        }
    }
    
    return node;
}

ASTNode* create_string_node(char* value) {
    ASTNode* node = calloc(1, sizeof(ASTNode));
    if (!node) return NULL;
    
    node->type = NODE_STRING;
    node->value.string_val = strdup(value);
    node->parent = NULL;
    node->node_id = get_next_id();
    
    return node;
}

ASTNode* create_integer_node(long value) {
    ASTNode* node = calloc(1, sizeof(ASTNode));
    if (!node) return NULL;
    
    node->type = NODE_INTEGER;
    node->value.int_val = value;
    node->parent = NULL;
    node->node_id = get_next_id();
    
    return node;
}

ASTNode* create_number_node(double value) {
    ASTNode* node = calloc(1, sizeof(ASTNode));
    if (!node) return NULL;
    
    node->type = NODE_NUMBER;
    node->value.num_val = value;
    node->parent = NULL;
    node->node_id = get_next_id();
    
    return node;
}

ASTNode* create_boolean_node(int value) {
    ASTNode* node = calloc(1, sizeof(ASTNode));
    if (!node) return NULL;
    
    node->type = NODE_BOOLEAN;
    node->value.bool_val = value;
    node->parent = NULL;
    node->node_id = get_next_id();
    
    return node;
}

ASTNode* create_null_node() {
    ASTNode* node = calloc(1, sizeof(ASTNode));
    if (!node) return NULL;
    
    node->type = NODE_NULL;
    node->parent = NULL;
    node->node_id = get_next_id();
    
    return node;
}

KeyValuePair* create_key_value_pair(char* key, ASTNode* value) {
    KeyValuePair* pair = calloc(1, sizeof(KeyValuePair));
    if (!pair) return NULL;
    
    pair->key = strdup(key);
    pair->value = value;
    
    return pair;
}

/* Get a value from an object by key name */
ASTNode* ast_object_get(ASTNode* obj, const char* key) {
    if (!obj || obj->type != NODE_OBJECT) return NULL;
    
    for (int i = 0; i < obj->value.object.pair_count; i++) {
        if (strcmp(obj->value.object.pairs[i]->key, key) == 0) {
            return obj->value.object.pairs[i]->value;
        }
    }
    
    return NULL;
}

/* Helper to add a key to a string buffer, with dynamic resizing */
void append_to_signature(char** sig, size_t* size, size_t* pos, const char* key) {
    size_t key_len = strlen(key);
    
    /* Check if we need to resize */
    while (*pos + key_len + 2 >= *size) {  /* +2 for delimiter and safety */
        *size *= 2;
        *sig = realloc(*sig, *size);
        if (!*sig) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
    }
    
    /* Add the key and a delimiter */
    strcpy(*sig + *pos, key);
    *pos += key_len;
    (*sig)[*pos] = ',';
    (*pos)++;
    (*sig)[*pos] = '\0';
}

/* Generate a signature that uniquely identifies object shapes with the same structure */
char* object_shape_signature(ASTNode* obj) {
    if (!obj || obj->type != NODE_OBJECT) return NULL;

    size_t size = 256;  /* Initial buffer size */
    size_t pos = 0;
    char* sig = malloc(size);
    if (!sig) return NULL;
    sig[0] = '\0';
    
    /* Sort keys alphabetically for consistent signatures */
    char** keys = malloc(obj->value.object.pair_count * sizeof(char*));
    if (!keys) {
        free(sig);
        return NULL;
    }
    
    /* Copy keys */
    for (int i = 0; i < obj->value.object.pair_count; i++) {
        keys[i] = obj->value.object.pairs[i]->key;
    }
    
    /* Simple bubble sort for keys (OK for small objects) */
    for (int i = 0; i < obj->value.object.pair_count - 1; i++) {
        for (int j = 0; j < obj->value.object.pair_count - i - 1; j++) {
            if (strcmp(keys[j], keys[j+1]) > 0) {
                char* temp = keys[j];
                keys[j] = keys[j+1];
                keys[j+1] = temp;
            }
        }
    }
    
    /* Build signature with sorted keys */
    for (int i = 0; i < obj->value.object.pair_count; i++) {
        append_to_signature(&sig, &size, &pos, keys[i]);
        
        /* Find the value for this key */
        ASTNode* value = NULL;
        for (int j = 0; j < obj->value.object.pair_count; j++) {
            if (strcmp(obj->value.object.pairs[j]->key, keys[i]) == 0) {
                value = obj->value.object.pairs[j]->value;
                break;
            }
        }
        
        /* Add type information to signature */
        if (value) {
            if (value->type == NODE_OBJECT) {
                append_to_signature(&sig, &size, &pos, "{}");
            } else if (value->type == NODE_ARRAY) {
                append_to_signature(&sig, &size, &pos, "[]");
                
                /* If array has elements, check first one's type */
                if (value->value.array.element_count > 0) {
                    ASTNode* first = value->value.array.elements[0];
                    if (first->type == NODE_OBJECT) {
                        /* Include object shape in array elements */
                        char* inner_sig = object_shape_signature(first);
                        if (inner_sig) {
                            append_to_signature(&sig, &size, &pos, inner_sig);
                            free(inner_sig);
                        }
                    }
                }
            } else {
                /* Add scalar type to signature */
                switch(value->type) {
                    case NODE_STRING: append_to_signature(&sig, &size, &pos, "s"); break;
                    case NODE_INTEGER: append_to_signature(&sig, &size, &pos, "i"); break;
                    case NODE_NUMBER: append_to_signature(&sig, &size, &pos, "n"); break;
                    case NODE_BOOLEAN: append_to_signature(&sig, &size, &pos, "b"); break;
                    case NODE_NULL: append_to_signature(&sig, &size, &pos, "0"); break;
                    default: break;
                }
            }
        }
    }
    
    free(keys);
    
    /* Remove trailing comma if it exists */
    if (pos > 0 && sig[pos-1] == ',') {
        sig[pos-1] = '\0';
    }
    
    return sig;
}

/* Check if an object matches a given signature */
int object_matches_signature(ASTNode* obj, const char* signature) {
    if (!obj || obj->type != NODE_OBJECT || !signature) return 0;
    
    char* obj_sig = object_shape_signature(obj);
    if (!obj_sig) return 0;
    
    int result = (strcmp(obj_sig, signature) == 0);
    free(obj_sig);
    
    return result;
}

/* Get unique ID for a node */
long get_node_id(ASTNode* node) {
    return node ? node->node_id : 0;
}

/* Helper functions to check node types */
int is_object(ASTNode* node) {
    return node && node->type == NODE_OBJECT;
}

int is_array(ASTNode* node) {
    return node && node->type == NODE_ARRAY;
}

int is_scalar(ASTNode* node) {
    if (!node) return 0;
    return node->type != NODE_OBJECT && node->type != NODE_ARRAY;
}

/* Print a node with proper indentation */
void print_ast_node(ASTNode* node, int indent) {
    if (!node) return;
    
    char indent_str[128] = {0};
    for (int i = 0; i < indent * 2; i++) indent_str[i] = ' ';
    
    switch(node->type) {
        case NODE_OBJECT:
            printf("%sOBJECT (%ld) {\n", indent_str, node->node_id);
            for (int i = 0; i < node->value.object.pair_count; i++) {
                KeyValuePair* pair = node->value.object.pairs[i];
                printf("%s  \"%s\": ", indent_str, pair->key);
                print_ast_node(pair->value, 0); // Key name provides indentation context
                printf("\n");
            }
            printf("%s}", indent_str);
            break;
            
        case NODE_ARRAY:
            printf("%sARRAY (%ld) [\n", indent_str, node->node_id);
            for (int i = 0; i < node->value.array.element_count; i++) {
                printf("%s  ", indent_str);
                print_ast_node(node->value.array.elements[i], indent + 1);
                printf("\n");
            }
            printf("%s]", indent_str);
            break;
            
        case NODE_STRING:
            printf("%s\"%s\"", indent_str, node->value.string_val);
            break;
            
        case NODE_INTEGER:
            printf("%s%ld", indent_str, node->value.int_val);
            break;
            
        case NODE_NUMBER:
            printf("%s%f", indent_str, node->value.num_val);
            break;
            
        case NODE_BOOLEAN:
            printf("%s%s", indent_str, node->value.bool_val ? "true" : "false");
            break;
            
        case NODE_NULL:
            printf("%snull", indent_str);
            break;
    }
}

/* Print the AST with indentation */
void print_ast(ASTNode* node, int indent) {
    print_ast_node(node, indent);
    printf("\n");
}

/* Free memory for an AST node and all its children */
void free_ast(ASTNode* node) {
    if (!node) return;
    
    switch(node->type) {
        case NODE_OBJECT:
            if (node->value.object.pairs) {
                for (int i = 0; i < node->value.object.pair_count; i++) {
                    if (node->value.object.pairs[i]) {
                        if (node->value.object.pairs[i]->key) {
                            free(node->value.object.pairs[i]->key);
                        }
                        free_ast(node->value.object.pairs[i]->value);
                        free(node->value.object.pairs[i]);
                    }
                }
                free(node->value.object.pairs);
            }
            break;
            
        case NODE_ARRAY:
            if (node->value.array.elements) {
                for (int i = 0; i < node->value.array.element_count; i++) {
                    free_ast(node->value.array.elements[i]);
                }
                free(node->value.array.elements);
            }
            break;
            
        case NODE_STRING:
            if (node->value.string_val) {
                free(node->value.string_val);
            }
            break;
            
        default:
            /* Other node types don't have allocated memory */
            break;
    }
    
    free(node);
}
