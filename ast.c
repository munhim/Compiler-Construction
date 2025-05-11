#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

static long nextnodeID = 1;

long getnid() {
    return nextnodeID++;
}
void resetNid() {
    nextnodeID = 1;
}

ASTNode* objnode(KeyValuePair** pairs, int count) {
    ASTNode* node = calloc(1, sizeof(ASTNode));
    if (!node) return NULL;
    node->type = nodeobj; 
    node->value.object.pairs = pairs;
    node->value.object.pairCount = count;
    node->parent = NULL;
    node->node_id = getnid();
    switch (pairs != NULL) {
        case 1: 
            int i = 0;
            while (i < count) {
                switch (pairs[i] != NULL && pairs[i]->value != NULL) {
                    case 1: 
                        pairs[i]->value->parent = node; 
                        break;
                    default:
                        break;
                }
                i++;
            }
            break;
        default:
            break;
    }
    return node;
}

ASTNode* arrnode(ASTNode** elements, int count) {
    ASTNode* node = calloc(1, sizeof(ASTNode));
    switch (node != NULL) {
        case 0:
            return NULL;
        default:
            break;
    }
    node->type = nodearr;
    node->value.array.elements = elements;
    node->value.array.elemCount = count;
    node->parent = NULL;
    node->node_id = getnid();
    switch (elements != NULL) {
        case 1: {
            int i = 0;
            while (i < count) {
                switch (elements[i] != NULL) {
                    case 1:
                        elements[i]->parent = node;
                        break;
                    default:
                        break;
                }
                i++;
            }
            break;
        }
        default:
            break;
    }
    return node;
}

ASTNode* strnode(char* value) {
    ASTNode* node = calloc(1, sizeof(ASTNode));
    switch (node != NULL) {
        case 0:
            return NULL;
        default:
            break;
    }
    node->type = nodestr;
    node->value.strVal = strdup(value);
    node->parent = NULL;
    node->node_id = getnid();
    return node;
}


ASTNode* intnode(long value) {
    ASTNode* node = calloc(1, sizeof(ASTNode));
    switch (node != NULL) {
        case 0:
            return NULL;
        default:
            break;
    }
    node->type = nodeint;
    node->value.intVal = value;
    node->parent = NULL;
    node->node_id = getnid();
    return node;
}

ASTNode* numnode(double value) {
    ASTNode* node = calloc(1, sizeof(ASTNode));
    switch (node != NULL) {
        case 0:
            return NULL;
        default:
            break;
    }
    node->type = nodenum;
    node->value.numVal = value;
    node->parent = NULL;
    node->node_id = getnid();
    return node;
}


ASTNode* boolnode(int value) {
    ASTNode* node = calloc(1, sizeof(ASTNode));
    switch (node != NULL) {
        case 0:
            return NULL;
        default:
            break;
    }
    node->type = nodebool;
    node->value.boolVal = value;
    node->parent = NULL;
    node->node_id = getnid();
    return node;
}


ASTNode* nullnode() {
    ASTNode* node = calloc(1, sizeof(ASTNode));
    switch (node != NULL) {
        case 0:
            return NULL;
        default:
            break;
    }
    node->type = nodenull;
    node->parent = NULL;
    node->node_id = getnid();
    return node;
}

KeyValuePair* createKVpair(char* key, ASTNode* value) {
    KeyValuePair* pair = calloc(1, sizeof(KeyValuePair));
    switch (pair != NULL) {
        case 0:
            return NULL;
        default:
            break;
    }
    pair->key = strdup(key);
    pair->value = value;
    return pair;
}

ASTNode* getbyname(ASTNode* obj, const char* key) {
    switch (obj && obj->type == nodeobj) {
        case 0:
            return NULL;
        default:
            break;
    }
    int i = 0;
    while (i < obj->value.object.pairCount) {
        if (strcmp(obj->value.object.pairs[i]->key, key) == 0) {
            return obj->value.object.pairs[i]->value;
        }
        i++;
    }
    return NULL;
}


void apptosig(char** sig, size_t* size, size_t* pos, const char* key) {
    size_t key_len = strlen(key);
    int need_resize = (*pos + key_len + 2 >= *size);
    while (need_resize) {
        *size *= 2;
        *sig = realloc(*sig, *size);

        switch (*sig != NULL) {
            case 0:
                fprintf(stderr, "Memory allocation failed\n");
                exit(1);
        }

        need_resize = (*pos + key_len + 2 >= *size);
    }
    strcpy(*sig + *pos, key);
    *pos += key_len;
    (*sig)[*pos] = ',';
    (*pos)++;
    (*sig)[*pos] = '\0';
}

char* getsig(ASTNode* obj) {
    switch (obj && obj->type == nodeobj) {
        case 0:
            return NULL;
    }
    size_t size = 256; 
    size_t pos = 0;
    char* sig = malloc(size);
    if (!sig) return NULL;
    sig[0] = '\0';
    int count = obj->value.object.pairCount;
    char** keys = malloc(count * sizeof(char*));
    if (!keys) {
        free(sig);
        return NULL;
    }
    int i = 0;
    while (i < count) {
        keys[i] = obj->value.object.pairs[i]->key;
        i++;
    }
    i = 0;
    while (i < count - 1) {
        int j = 0;
        while (j < count - i - 1) {
            if (strcmp(keys[j], keys[j+1]) > 0) {
                char* temp = keys[j];
                keys[j] = keys[j+1];
                keys[j+1] = temp;
            }
            j++;
        }
        i++;
    }
    i = 0;
    while (i < count) {
        apptosig(&sig, &size, &pos, keys[i]);
        ASTNode* value = NULL;
        int j = 0;
        while (j < count) {
            if (strcmp(obj->value.object.pairs[j]->key, keys[i]) == 0) {
                value = obj->value.object.pairs[j]->value;
                break;
            }
            j++;
        }
        if (value) {
            switch (value->type) {
                case nodeobj:
                    apptosig(&sig, &size, &pos, "{}");
                    break;
                case nodearr:
                    apptosig(&sig, &size, &pos, "[]");
                    if (value->value.array.elemCount > 0) {
                        ASTNode* first = value->value.array.elements[0];
                        switch (first->type) {
                            case nodeobj: {
                                char* inner_sig = getsig(first);
                                if (inner_sig) {
                                    apptosig(&sig, &size, &pos, inner_sig);
                                    free(inner_sig);
                                }
                                break;
                            }
                            default:
                                break;
                        }
                    }
                    break;
                case nodestr:
                    apptosig(&sig, &size, &pos, "s");
                    break;
                case nodeint:
                    apptosig(&sig, &size, &pos, "i");
                    break;
                case nodenum:
                    apptosig(&sig, &size, &pos, "n");
                    break;
                case nodebool:
                    apptosig(&sig, &size, &pos, "b");
                    break;
                case nodenull:
                    apptosig(&sig, &size, &pos, "0");
                    break;
                default:
                    break;
            }
        }
        i++;
    }
    free(keys);
    if (pos > 0 && sig[pos - 1] == ',') {
        sig[pos - 1] = '\0';
    }
    return sig;
}

int matches(ASTNode* obj, const char* signature) {
    if (!obj || obj->type != nodeobj || !signature) return 0;
    
    char* obj_sig = getsig(obj);
    if (!obj_sig) return 0;
    
    int result = (strcmp(obj_sig, signature) == 0);
    free(obj_sig);
    
    return result;
}

long getnodeID(ASTNode* node) {
    return node ? node->node_id : 0;
}

int isobj(ASTNode* node) {
    return node && node->type == nodeobj;
}

int isArray(ASTNode* node) {
    return node && node->type == nodearr;
}

int scalar(ASTNode* node) {
    if (!node) return 0;
    return node->type != nodeobj && node->type != nodearr;
}

void printnodeast(ASTNode* node, int indent, char* prefix) {
    if (!node) return;
    char indent_str[256] = {0};
    char tree_prefix[256] = {0};
    switch(prefix != NULL) {
        case 1:
            strcpy(tree_prefix, prefix);
            break;
        case 0:
            break;
    }
    for (int i = 0; i < indent; i++) {
        if (i == indent - 1) {
            strcat(tree_prefix, "└── ");
        } else {
            strcat(tree_prefix, "    ");
        }
    }
    for (int i = 0; i < indent * 4; i++) {
        indent_str[i] = ' ';
    }
    switch(node->type) {
        case nodeobj:
            printf("%sObject (id=%ld) {\n", tree_prefix, node->node_id);
            
            int i = 0;
            while (i < node->value.object.pairCount) {
                KeyValuePair* pair = node->value.object.pairs[i];
                char child_prefix[256] = {0};
                strcpy(child_prefix, indent_str);
                switch(i == node->value.object.pairCount - 1) {
                    case 1:
                        strcat(child_prefix, "└── ");
                        break;
                    default:
                        strcat(child_prefix, "├── ");
                        break;
                }
                printf("%s\"%s\": ", child_prefix, pair->key);
                if (scalar(pair->value)) {
                    printnodeast(pair->value, 0, NULL);
                    printf("\n");
                } else {
                    printf("\n");
                    char nested_prefix[256] = {0};
                    strcpy(nested_prefix, indent_str);
                    switch(i == node->value.object.pairCount - 1) {
                        case 1:
                            strcat(nested_prefix, "    ");
                            break;
                        default:
                            strcat(nested_prefix, "│   ");
                            break;
                    }
                    printnodeast(pair->value, 1, nested_prefix);
                }
                i++;
            }
            if (indent > 0) {
                printf("%s}\n", indent_str);
            } else if (prefix) {
                printf("%s}\n", prefix);
            } else {
                printf("}\n");
            }
            break;
        case nodearr:
            printf("%sArray (id=%ld) [\n", tree_prefix, node->node_id);
            int j = 0;
            while (j < node->value.array.elemCount) {
                char child_prefix[256] = {0};
                strcpy(child_prefix, indent_str);
                switch(j == node->value.array.elemCount - 1) {
                    case 1:
                        printf("%s└── [%d]: ", indent_str, j);
                        strcat(child_prefix, "    ");
                        break;
                    default:
                        printf("%s├── [%d]: ", indent_str, j);
                        strcat(child_prefix, "│   ");
                        break;
                }
                if (scalar(node->value.array.elements[j])) {
                    printnodeast(node->value.array.elements[j], 0, NULL);
                    printf("\n");
                } else {
                    printf("\n");
                    printnodeast(node->value.array.elements[j], 1, child_prefix);
                }
                j++;
            } 
            if (indent > 0) {
                printf("%s]\n", indent_str);
            } else if (prefix) {
                printf("%s]\n", prefix);
            } else {
                printf("]\n");
            }
            break;
        case nodestr:
            printf("%s\"%s\"", tree_prefix, node->value.strVal);
            break;
        case nodeint:
            printf("%s%ld", tree_prefix, node->value.intVal);
            break;
        case nodenum:
            printf("%s%f", tree_prefix, node->value.numVal);
            break;
        case nodebool:
            printf("%s%s", tree_prefix, node->value.boolVal ? "true" : "false");
            break;
        case nodenull:
            printf("%snull", tree_prefix);
            break;
    }
}

void printast(ASTNode* node, int indent) {
    printf("\n===== AST Tree View =====\n");
    printnodeast(node, indent, NULL);
}

void deleteast(ASTNode* node) {
    if (!node) return; 
    switch(node->type) {
        case nodeobj:
            if (node->value.object.pairs) {
                int i = 0;
                while (i < node->value.object.pairCount) {
                    if (node->value.object.pairs[i]) {
                        if (node->value.object.pairs[i]->key) {
                            free(node->value.object.pairs[i]->key);
                        }
                        deleteast(node->value.object.pairs[i]->value);
                        free(node->value.object.pairs[i]);
                    }
                    i++;
                }
                free(node->value.object.pairs);
            }
            break;    
        case nodearr:
            if (node->value.array.elements) {
                int i = 0;
                while (i < node->value.array.elemCount) {
                    deleteast(node->value.array.elements[i]);
                    i++;
                }
                free(node->value.array.elements);
            }
            break;
            
        case nodestr:
            if (node->value.strVal) {
                free(node->value.strVal);
            }
            break;
            
        default:
            break;
    } 
    free(node);
}

