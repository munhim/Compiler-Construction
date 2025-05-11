#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "schema.h"

/* Create a new schema */
Schema* makeSchema() {
    Schema* schema = calloc(1, sizeof(Schema));
    return schema;
}

void delSchema(Schema* schema) {
    switch (schema == NULL) {
        case 1:
            return;
        default:
            break;
    }
    int i = 0;
    while (i < schema->table_count) {
        Table* table = &schema->tables[i];

        switch (table->name == NULL) {
            case 0:
                free(table->name);
                break;
        }
        switch (table->signature == NULL) {
            case 0:
                free(table->signature);
                break;
        }
        int j = 0;
        while (j < table->column_count) {
            Column* col = &table->columns[j];

            switch (col->name == NULL) {
                case 0:
                    free(col->name);
                    break;
            }
            switch (col->references == NULL) {
                case 0:
                    free(col->references);
                    break;
            }

            j++;
        }
        i++;
    }
    free(schema);
}


int addT(Schema* schema, const char* name, int is_junction, int is_child) {
    switch (schema->table_count >= MAX_TABLES) {
        case 1:
            fprintf(stderr, "Maximum number of tables exceeded\n");
            return -1;
        default:
            break;
    }
    int table_index = schema->table_count++;
    schema->tables[table_index].name = strdup(name);
    schema->tables[table_index].column_count = 0;
    schema->tables[table_index].signature = NULL;
    schema->tables[table_index].is_junction = is_junction;
    schema->tables[table_index].is_child = is_child;
    addC(schema, table_index, "id", COL_ID, NULL);
    return table_index;
}

void addC(Schema* schema, int table_index, const char* name, ColumnType type, const char* references) {
    switch (table_index < 0 || table_index >= schema->table_count) {
        case 1:
            return;
        default:
            break;
    }
    Table* table = &schema->tables[table_index];

    switch (table->column_count >= MAX_COLUMNS) {
        case 1:
            fprintf(stderr, "Maximum number of columns exceeded for table %s\n", table->name);
            return;
        default:
            break;
    }
    Column* col = &table->columns[table->column_count++];
    col->name = strdup(name);
    col->type = type;
    col->references = references ? strdup(references) : NULL;
}

int exists(Schema* schema, const char* name) {
    for (int i = 0; i < schema->table_count; i++) {
        if (strcmp(schema->tables[i].name, name) == 0) {
            return 1;
        }
    }
    return 0;
}

int colexist(Schema* schema, int table_index, const char* name) {
    switch (!schema || table_index < 0 || table_index >= schema->table_count) {
        case 1:
            return 0;
        default:
            break;
    }
    Table* table = &schema->tables[table_index];
    int i = 0;
    while (i < table->column_count) {
        switch (strcmp(table->columns[i].name, name) == 0) {
            case 1:
                return 1;
            default:
                break;
        }
        i++;
    }
    return 0;
}

int tableexists(Schema* schema, const char* signature) {
    switch (!signature) {
        case 1:
            return 0;
        default:
            break;
    }
    int i = 0;
    while (i < schema->table_count) {
        switch (schema->tables[i].signature && strcmp(schema->tables[i].signature, signature) == 0) {
            case 1:
                return 1;
            default:
                break;
        }
        i++;
    }
    return 0;
}

int gettablei(Schema* schema, const char* name) {
    int i = 0;
    while (i < schema->table_count) {
        switch (strcmp(schema->tables[i].name, name) == 0) {
            case 1:
                return i;
            default:
                break;
        }
        i++;
    }
    return -1;
}

int getibysig(Schema* schema, const char* signature) {
    if (!signature) return -1;
    int i = 0;
    while (i < schema->table_count) {
        switch (schema->tables[i].signature && strcmp(schema->tables[i].signature, signature) == 0) {
            case 1:
                return i;
            default:
                break;
        }
        i++;
    }
    return -1;
}

char* tolowercase(const char* str) {
    size_t len = strlen(str);
    char* lower = malloc(len + 1);
    if (!lower) return NULL;
    
    for (size_t i = 0; i < len; i++) {
        lower[i] = tolower(str[i]);
    }
    lower[len] = '\0';
    return lower;
}

int endswiths(const char* str) {
    size_t len = strlen(str);
    return len > 0 && str[len - 1] == 's';
}

char* predicttablename(ASTNode* obj, const char* parent_key, const char* default_name) {
    if (parent_key) {
        switch (1) {
            case 1:
                if (strcmp(parent_key, "author") == 0 || strcmp(parent_key, "comments") == 0) {
                    ASTNode* uid_node = getbyname(obj, "uid");
                    if (uid_node && uid_node->type == nodestr) {
                        return strdup("users");
                    }
                }
                break;
            case 2:
                if (strcmp(parent_key, "posts") == 0 || 
                    (obj->parent && obj->parent->type == nodeobj && 
                     getbyname(obj->parent, "postId") != NULL)) {
                    return strdup("posts");
                }
                break;
            case 3:
                if (strcmp(parent_key, "comments") == 0) {
                    return strdup("comments");
                }
                break;
            case 4:
                if (strcmp(parent_key, "customer") == 0) {
                    return strdup("customers");
                }
                break;
            case 5:
                if (strcmp(parent_key, "items") == 0) {
                    if (obj->parent && obj->parent->type == nodeobj) {
                        if (getbyname(obj->parent, "orderId") != NULL || 
                            getbyname(obj->parent, "order_id") != NULL) {
                            return strdup("orderitems");
                        }
                        if ((getbyname(obj->parent, "total") != NULL ||
                             getbyname(obj->parent, "amount") != NULL) &&
                            (getbyname(obj->parent, "date") != NULL || 
                             getbyname(obj->parent, "orderDate") != NULL)) {
                            return strdup("orderitems");
                        }
                    }
                    if (obj && obj->type == nodeobj) {
                        int has_sku = (getbyname(obj, "sku") != NULL);
                        int has_qty = (getbyname(obj, "qty") != NULL || 
                                      getbyname(obj, "quantity") != NULL);

                        if (has_sku && has_qty) {
                            return strdup("orderitems");
                        }
                    }
                }
                break;
        }
    }
    if (obj && getbyname(obj, "postId") != NULL) {
        return strdup("posts");
    }
    const char* id_fields[] = {"type", "kind", "name", "category", "class", NULL};
    if (isobj(obj)) {
        int i = 0;
        while (id_fields[i] != NULL) {
            ASTNode* id_node = getbyname(obj, id_fields[i]);
            if (id_node && id_node->type == nodestr) {
                char* name = tolowercase(id_node->value.strVal);
                if (name[0] && !endswiths(name)) {
                    char* plural = malloc(strlen(name) + 2);
                    sprintf(plural, "%ss", name);
                    free(name);
                    return plural;
                }
                return name;
            }
            i++;
        }
    }
    if (parent_key) {
        char* name = tolowercase(parent_key);
        if (strcmp(name, "author") == 0) {
            free(name);
            return strdup("users");
        }
        return name;
    }

    return strdup(default_name);
}

void processScalar(Schema* schema, ASTNode* array, const char* parent_table, 
                           long parent_id, const char* parent_key) {
    if (!isArray(array) || !parent_table || !parent_key) return;

    /* Create a junction table for this array */
    char table_name[256];
    sprintf(table_name, "%s", parent_key);

    int table_index;
    if (!exists(schema, table_name)) {
        table_index = addT(schema, table_name, 1, 0);

        /* Add junction table columns */
        char fk_name[256];
        sprintf(fk_name, "%s_id", parent_table);
        addC(schema, table_index, fk_name, COL_FOREIGN_KEY, parent_table);
        addC(schema, table_index, "index", COL_INDEX, NULL);
        addC(schema, table_index, "value", COL_STRING, NULL);
    } else {
        table_index = gettablei(schema, table_name);
    }
}

void processOrderItems(Schema* schema, ASTNode* value) {
    int items_table_index = -1;
    if (!exists(schema, "order_items")) {
        items_table_index = addT(schema, "order_items", 0, 1);
        addC(schema, items_table_index, "order_id", COL_FOREIGN_KEY, "orders");
        addC(schema, items_table_index, "seq", COL_INDEX, NULL);
    } else {
        items_table_index = gettablei(schema, "order_items");
    }
    for (int j = 0; j < value->value.array.elemCount; j++) {
        ASTNode* item = value->value.array.elements[j];
        if (isobj(item)) {
            for (int k = 0; k < item->value.object.pairCount; k++) {
                KeyValuePair* item_pair = item->value.object.pairs[k];
                if (scalar(item_pair->value)) {
                    ColumnType col_type;
                    switch(item_pair->value->type) {
                        case nodestr: col_type = COL_STRING; break;
                        case nodeint: col_type = COL_INTEGER; break;
                        case nodenum: col_type = COL_NUMBER; break;
                        case nodebool: col_type = COL_BOOLEAN; break;
                        default: col_type = COL_STRING; break;
                    }
                    if (!colexist(schema, items_table_index, item_pair->key)) {
                        addC(schema, items_table_index, item_pair->key, col_type, NULL);
                    }
                }
            }
        }
    }
}

void processArray(Schema* schema, ASTNode* value, ASTNode* obj, int table_index, const char* key) {
    if (value->value.array.elemCount > 0) {
        ASTNode* first = value->value.array.elements[0];
        if (scalar(first)) {
            processScalar(schema, value, schema->tables[table_index].name, obj->node_id, key);
        } else if (isobj(first)) {
            if (strcmp(key, "items") == 0 && strcmp(schema->tables[table_index].name, "orders") == 0) {
                processOrderItems(schema, value);
            } else {
                for (int j = 0; j < value->value.array.elemCount; j++) {
                    ASTNode* item = value->value.array.elements[j];
                    if (isobj(item)) {
                        processobj(schema, item, schema->tables[table_index].name, obj->node_id, j);
                    }
                }
            }
        }
    }
}

void addColumnsForObject(Schema* schema, ASTNode* obj, int table_index) {
    for (int i = 0; i < obj->value.object.pairCount; i++) {
        KeyValuePair* pair = obj->value.object.pairs[i];
        ASTNode* value = pair->value;

        if (scalar(value)) {
            if (strcmp(schema->tables[table_index].name, "comments") == 0 && strcmp(pair->key, "uid") == 0) {
                continue;
            }
            ColumnType col_type;
            switch (value->type) {
                case nodestr: col_type = COL_STRING; break;
                case nodeint: col_type = COL_INTEGER; break;
                case nodenum: col_type = COL_NUMBER; break;
                case nodebool: col_type = COL_BOOLEAN; break;
                default: col_type = COL_STRING; break;
            }
            addC(schema, table_index, pair->key, col_type, NULL);
        } else if (isobj(value)) {
            processobj(schema, value, schema->tables[table_index].name, 0, -1);
            char fk_name[256];
            sprintf(fk_name, "%s_id", pair->key);
            addC(schema, table_index, fk_name, COL_FOREIGN_KEY, pair->key);
        } else if (isArray(value)) {
            processArray(schema, value, obj, table_index, pair->key);
        }
    }
}

char* predicttablename_for_child(ASTNode* obj, const char* parent_key, const char* parent_table) {
    char* table_name = predicttablename(obj, parent_key, parent_table);

    if (strcmp(parent_key, "items") == 0) {
        int has_sku = 0;
        int has_qty_or_quantity = 0;
        int has_price = 0;

        for (int i = 0; i < obj->value.object.pairCount; i++) {
            const char* key = obj->value.object.pairs[i]->key;
            if (strcmp(key, "sku") == 0) has_sku = 1;
            if (strcmp(key, "qty") == 0 || strcmp(key, "quantity") == 0) has_qty_or_quantity = 1;
            if (strcmp(key, "price") == 0) has_price = 1;
        }

        if (has_sku && (has_qty_or_quantity || has_price)) {
            free(table_name);
            table_name = strdup("order_items");
        }
    }
    return table_name;
}

char* determineTableName(Schema* schema, ASTNode* obj, const char* parent_table, int array_index) {
    char* table_name = NULL;
    if (!parent_table) {
        if (getbyname(obj, "postId")) {
            table_name = strdup("posts");
        } else if (getbyname(obj, "orderId") || getbyname(obj, "order_id")) {
            table_name = strdup("orders");
        } else if (getbyname(obj, "items") && (getbyname(obj, "total") || getbyname(obj, "customer"))) {
            table_name = strdup("orders");
        } else {
            table_name = strdup("root");
        }
    } else {
        const char* parent_key = NULL;
        if (obj->parent && obj->parent->type == nodeobj) {
            for (int i = 0; i < obj->parent->value.object.pairCount; i++) {
                if (obj->parent->value.object.pairs[i]->value == obj) {
                    parent_key = obj->parent->value.object.pairs[i]->key;
                    break;
                }
            }
        }
        if (parent_key) {
            table_name = predicttablename_for_child(obj, parent_key, parent_table);
        } else {
            table_name = strdup(parent_table);
        }
    }
    return table_name;
}

void processAuthor(Schema* schema, ASTNode* obj, int posts_table_index) {
    ASTNode* author = getbyname(obj, "author");
    if (author && isobj(author)) {
        addC(schema, posts_table_index, "author_id", COL_FOREIGN_KEY, "users");
        if (!exists(schema, "users")) {
            int users_table_index = addT(schema, "users", 0, 0);
            addC(schema, users_table_index, "uid", COL_STRING, NULL);
            addC(schema, users_table_index, "name", COL_STRING, NULL);
        }
        processobj(schema, author, "posts", 0, -1);
    }
}

void processComments(Schema* schema, ASTNode* obj) {
    ASTNode* comments = getbyname(obj, "comments");
    if (comments && isArray(comments) && comments->value.array.elemCount > 0) {
        if (!exists(schema, "comments")) {
            int comments_table_index = addT(schema, "comments", 0, 1);
            addC(schema, comments_table_index, "post_id", COL_FOREIGN_KEY, "posts");
            addC(schema, comments_table_index, "seq", COL_INDEX, NULL);
            addC(schema, comments_table_index, "user_id", COL_FOREIGN_KEY, "users");
            addC(schema, comments_table_index, "text", COL_STRING, NULL);
        }
        for (int i = 0; i < comments->value.array.elemCount; i++) {
            ASTNode* comment = comments->value.array.elements[i];
            if (isobj(comment)) {
                processobj(schema, comment, "comments", 0, i);
            }
        }
    }
}

void processPostsRoot(Schema* schema, ASTNode* obj) {
    int posts_table_index = -1;
    if (!exists(schema, "posts")) {
        posts_table_index = addT(schema, "posts", 0, 0);
        addC(schema, posts_table_index, "postId", COL_INTEGER, NULL);
        processAuthor(schema, obj, posts_table_index);
        processComments(schema, obj);
    }
}

void processobj(Schema* schema, ASTNode* obj, const char* parent_table, 
                long parent_id, int array_index) {
    if (!isobj(obj)) return;
    if (getbyname(obj, "postId") != NULL && !parent_table) {
        processPostsRoot(schema, obj);
        return;
    }
    char* signature = getsig(obj);
    int table_index = -1;
    char* table_name = NULL;
    if (signature) {
        table_index = getibysig(schema, signature);
    }
    if (table_index == -1) {
        table_name = determineTableName(schema, obj, parent_table, array_index);
        if (strcmp(table_name, "users") == 0 && exists(schema, "users")) {
            free(table_name);
            free(signature);
            return;
        }
        int is_child = (array_index >= 0);
        table_index = addT(schema, table_name, 0, is_child);
        free(table_name);
        schema->tables[table_index].signature = signature;
        if (is_child && parent_table) {
            char fk_name[256];
            sprintf(fk_name, "%s_id", parent_table);
            addC(schema, table_index, fk_name, COL_FOREIGN_KEY, parent_table);
            addC(schema, table_index, "seq", COL_INDEX, NULL);

            if (strcmp(schema->tables[table_index].name, "comments") == 0) {
                addC(schema, table_index, "user_id", COL_FOREIGN_KEY, "users");
            }
        }
        addColumnsForObject(schema, obj, table_index);
    } else {
        free(signature); 
    }
}

void processArrayItems(Schema* schema, ASTNode* ast) {
    for (int i = 0; i < ast->value.array.elemCount; i++) {
        ASTNode* item = ast->value.array.elements[i];
        if (isobj(item)) {
            processobj(schema, item, "root", 0, i);
        }
    }
}

void handleArray(Schema* schema, ASTNode* ast) {
    if (ast->value.array.elemCount > 0) {
        ASTNode* first = ast->value.array.elements[0];
        if (isobj(first)) {
            processArrayItems(schema, ast);
        } else {
            processScalar(schema, ast, "root", 0, "values");
        }
    }
}

void genSchema(Schema* schema, ASTNode* ast) {
    if (!schema || !ast) return;

    if (isobj(ast)) {
        processobj(schema, ast, NULL, 0, -1);
    } else if (isArray(ast)) {
        handleArray(schema, ast);
    }
}


void printColumn(Column* col) {
    printf("  %s: ", col->name);
    switch (col->type) {
        case COL_ID: 
            printf("ID"); 
            break;
        case COL_FOREIGN_KEY: 
            printf("FK -> %s", col->references ? col->references : "unknown"); 
            break;
        case COL_INDEX: 
            printf("INDEX"); 
            break;
        case COL_STRING: 
            printf("STRING"); 
            break;
        case COL_INTEGER: 
            printf("INTEGER"); 
            break;
        case COL_NUMBER: 
            printf("NUMBER"); 
            break;
        case COL_BOOLEAN: 
            printf("BOOLEAN"); 
            break;
        default: 
            printf("UNKNOWN"); 
            break;
    }
    printf("\n");
}

void printTable(Table* table) {
    printf("Table: %s", table->name);
    if (table->is_junction) printf(" (junction)");
    if (table->is_child) printf(" (child)");
    printf("\n");
    
    for (int j = 0; j < table->column_count; j++) {
        printColumn(&table->columns[j]);
    }
}

void printschema(Schema* schema) {
    if (!schema) return;
    printf("Schema (%d tables):\n", schema->table_count);
    
    for (int i = 0; i < schema->table_count; i++) {
        printTable(&schema->tables[i]);
    }
}

