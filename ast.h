#ifndef AST_H
#define AST_H
#include <stdio.h>

typedef enum {
    nodeobj,
    nodearr,
    nodestr,
    nodeint,
    nodenum,
    nodebool,
    nodenull
} NodeType;

typedef struct ASTNode ASTNode;

typedef struct KeyValuePair {
    char* key;
    ASTNode* value;
} KeyValuePair;

struct ASTNode {
    NodeType type;
    union {
        struct {
            KeyValuePair** pairs;
            int pairCount;
        } object;
        struct {
            ASTNode** elements;
            int elemCount;
        } array;
        char* strVal;
        long intVal;
        double numVal;
        int boolVal;
    } value;
    ASTNode* parent;
    long node_id;
};

ASTNode* objnode(KeyValuePair** pairs, int count);
ASTNode* arrnode(ASTNode** elements, int count);
ASTNode* strnode(char* value);
ASTNode* intnode(long value);
ASTNode* numnode(double value);
ASTNode* boolnode(int value);
ASTNode* nullnode();
int isobj(ASTNode* node);
int isArray(ASTNode* node);
int scalar(ASTNode* node);
KeyValuePair* createKVpair(char* key, ASTNode* value);
void printnodeast(ASTNode* node, int indent, char* prefix);
void printast(ASTNode* node, int indent);
void deleteast(ASTNode* node);
ASTNode* getbyname(ASTNode* obj, const char* key);
char* getsig(ASTNode* obj);
long getnodeID(ASTNode* node);
long getnid();
int matches(ASTNode* obj, const char* signature);
#endif 
