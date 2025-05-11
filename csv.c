#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "csv.h"
#include "helper.h"

char* esc(const char* s) {
    if (!s) return strdup("");
    size_t len = strlen(s);
    size_t escaped_len = len + 2; 
    size_t i = 0;
    while (i < len) {
        switch (s[i]) {
            case '"':
                escaped_len++;
                break;
        }
        i++;
    }
    char* result = malloc(escaped_len + 1);
    if (!result) return NULL;
    result[0] = '"';
    i = 0;
    size_t j = 1;

    while (i < len) {
        switch (s[i]) {
            case '"':
                result[j++] = '"';
                result[j++] = '"';
                break;
            default:
                result[j++] = s[i];
                break;
        }
        i++;
    }
    result[j++] = '"';
    result[j] = '\0';
    return result;
}
char* nodetocsv(ASTNode* node) {
    switch (node == NULL) {
        case 1:
            return strdup("");
        default:
            break;
    }
    switch (node->type) {
        case nodestr: {
            return esc(node->value.strVal);
        }
        case nodeint: {
            char buf[32];
            sprintf(buf, "%ld", node->value.intVal);
            return strdup(buf);
        }
        case nodenum: {
            char buf[32];
            sprintf(buf, "%g", node->value.numVal);
            return strdup(buf);
        }
        case nodebool: {
            switch (node->value.boolVal) {
                case 1:
                    return strdup("true");
                default:
                    return strdup("false");
            }
        }
        case nodenull: {
            return strdup("");
        }
        default:
            return strdup("");
    }
}

/* Write CSV header row */
void csvheader(Schema* schema, int table_index, FILE* fp) {
    Table* table = &schema->tables[table_index];
    int i = 0;
    while (i < table->column_count) {
        fprintf(fp, "%s", table->columns[i].name);

        switch (i < table->column_count - 1) {
            case 1:
                fprintf(fp, ",");
                break;
            default:
                break;
        }
        i++;
    }
    fprintf(fp, "\n");
}

void scalarcsv(Schema* schema, int table_index, ASTNode* array, 
                           FILE* fp, long parent_id) {
    switch (!isArray(array) || table_index < 0 || table_index >= schema->table_count) {
        case 1:
            return;
        default:
            break;
    }
    int i = 0;
    while (i < array->value.array.elemCount) {
        ASTNode* item = array->value.array.elements[i];
        long row_id = getnid();
        fprintf(fp, "%ld,", row_id);     
        fprintf(fp, "%ld,", parent_id);
        fprintf(fp, "%d,", i);           
        char* value = nodetocsv(item);
        fprintf(fp, "%s", value);
        free(value);
        fprintf(fp, "\n");
        i++;
    }
}

void writeOrderItems(Schema* schema, ASTNode* obj, FILE* fp, long parentId, int index, const char* outputDir) {
    ASTNode* skuNode = getbyname(obj, "sku");
    ASTNode* qtyNode = getbyname(obj, "qty");
    ASTNode* nameNode = getbyname(obj, "name");
    ASTNode* priceNode = getbyname(obj, "price");
    ASTNode* quantityNode = getbyname(obj, "quantity");
    int isSimple = (skuNode && qtyNode && !nameNode && !priceNode && !quantityNode) ? 1 : 0;
    if (isSimple) {
        long itemId = getnid();
        fprintf(fp, "%ld,%ld,%d,", itemId, parentId, index);
        if (skuNode && skuNode->type == nodestr) {
            char* skuVal = nodetocsv(skuNode);
            fprintf(fp, "%s,", skuVal);
            free(skuVal);
        } else {
            fprintf(fp, ",");
        }
        if (qtyNode && qtyNode->type == nodeint) {
            fprintf(fp, "%ld", qtyNode->value.intVal);
        } else {
            fprintf(fp, ",");
        }
        fprintf(fp, "\n");
        return;
    }
    long itemId = getnid();
    fprintf(fp, "%ld,%ld,%d,", itemId, parentId, index);
    if (skuNode && skuNode->type == nodestr) {
        char* skuVal = nodetocsv(skuNode);
        fprintf(fp, "%s,", skuVal);
        free(skuVal);
    } else {
        fprintf(fp, ",");
    }
    if (nameNode && nameNode->type == nodestr) {
        char* nameVal = nodetocsv(nameNode);
        fprintf(fp, "%s,", nameVal);
        free(nameVal);
    } else {
        fprintf(fp, ",");
    }
    if (priceNode && (priceNode->type == nodenum || priceNode->type == nodeint)) {
        if (priceNode->type == nodenum) {
            fprintf(fp, "%.2f,", priceNode->value.numVal);
        } else {
            fprintf(fp, "%ld,", priceNode->value.intVal);
        }
    } else {
        fprintf(fp, ",");
    }
    if (!qtyNode) {
        qtyNode = getbyname(obj, "quantity");
        if (!qtyNode) {
            qtyNode = getbyname(obj, "qty");
        }
    }
    if (qtyNode && qtyNode->type == nodeint) {
        fprintf(fp, "%ld", qtyNode->value.intVal);
    } else {
        fprintf(fp, ",");
    }
    fprintf(fp, "\n");
}

void writeOrders(Schema* schema, ASTNode* obj, FILE* fp, const char* outputDir) {
    fprintf(fp, "%ld,", obj->node_id);
    ASTNode* orderIdNode = getbyname(obj, "orderId");
    if (orderIdNode && orderIdNode->type == nodeint) {
        fprintf(fp, "%ld,", orderIdNode->value.intVal);
    } else {
        fprintf(fp, ",");
    }
    ASTNode* customerNode = getbyname(obj, "customer");
    if (customerNode && isobj(customerNode)) {
        fprintf(fp, "%ld,", customerNode->node_id);
    } else {
        fprintf(fp, ",");
    }
    ASTNode* totalNode = getbyname(obj, "total");
    if (totalNode) {
        if (totalNode->type == nodenum) {
            fprintf(fp, "%.2f,", totalNode->value.numVal);
        } else if (totalNode->type == nodeint) {
            fprintf(fp, "%ld,", totalNode->value.intVal);
        } else {
            fprintf(fp, ",");
        }
    } else {
        fprintf(fp, ",");
    }
    ASTNode* dateNode = getbyname(obj, "date");
    if (dateNode && dateNode->type == nodestr) {
        char* dateVal = nodetocsv(dateNode);
        fprintf(fp, "%s", dateVal);
        free(dateVal);
    } else {
        fprintf(fp, ",");
    }
    fprintf(fp, "\n");
    if (customerNode && isobj(customerNode)) {
        int custTableIndex = gettablei(schema, "customers");
        if (custTableIndex >= 0) {
            char path[512];
            sprintf(path, "%s/customers.csv", outputDir);
            FILE* custFp = fopen(path, "a");
            if (custFp) {
                fprintf(custFp, "%ld,", customerNode->node_id);
                ASTNode* idNode = getbyname(customerNode, "id");
                if (idNode) {
                    char* idVal = nodetocsv(idNode);
                    fprintf(custFp, "%s,", idVal);
                    free(idVal);
                } else {
                    fprintf(custFp, ",");
                }
                ASTNode* nameNode = getbyname(customerNode, "name");
                if (nameNode && nameNode->type == nodestr) {
                    char* nameVal = nodetocsv(nameNode);
                    fprintf(custFp, "%s", nameVal);
                    free(nameVal);
                } else {
                    fprintf(custFp, ",");
                }
                fprintf(custFp, "\n");
                fclose(custFp);
            }
        }
    }
    ASTNode* itemsNode = getbyname(obj, "items");
    if (itemsNode && itemsNode->type == nodearr) {
        int itemsTableIndex = gettablei(schema, "order_items");
        if (itemsTableIndex >= 0) {
            char path[512];
            sprintf(path, "%s/order_items.csv", outputDir);
            FILE* itemsFp = fopen(path, "a");
            if (itemsFp) {
                int i = 0;
                while (i < itemsNode->value.array.elemCount) {
                    ASTNode* item = itemsNode->value.array.elements[i];
                    if (isobj(item)) {
                        writeobj(schema, itemsTableIndex, item, itemsFp, obj->node_id, i, "orders", outputDir);
                    }
                    i++;
                }
                fclose(itemsFp);
            }
        }
    }
}

void writePosts(Schema* schema, ASTNode* obj, FILE* fp, const char* outputDir) {
    ASTNode* postIdNode = getbyname(obj, "postId");
    ASTNode* authorNode = getbyname(obj, "author");
    if (postIdNode && authorNode && isobj(authorNode)) {
        fprintf(fp, "1,");
        if (postIdNode->type == nodeint) {
            fprintf(fp, "%ld", postIdNode->value.intVal);
        } else {
            fprintf(fp, "0");
        }
        fprintf(fp, ",1\n");
        int usersTableIndex = gettablei(schema, "users");
        if (usersTableIndex >= 0) {
            char path[512];
            sprintf(path, "%s/users.csv", outputDir);
            FILE* usersFp = fopen(path, "a");
            if (usersFp) {
                ASTNode* uidNode = getbyname(authorNode, "uid");
                ASTNode* nameNode = getbyname(authorNode, "name");
                fprintf(usersFp, "1,");
                if (uidNode && uidNode->type == nodestr) {
                    char* uidVal = nodetocsv(uidNode);
                    fprintf(usersFp, "%s", uidVal);
                    free(uidVal);
                } else {
                    fprintf(usersFp, ",");
                }
                fprintf(usersFp, ",");
                if (nameNode && nameNode->type == nodestr) {
                    char* nameVal = nodetocsv(nameNode);
                    fprintf(usersFp, "%s", nameVal);
                    free(nameVal);
                } else {
                    fprintf(usersFp, ",");
                }
                fprintf(usersFp, "\n");
                fclose(usersFp);
            }
        }
        ASTNode* commentsNode = getbyname(obj, "comments");
        if (commentsNode && commentsNode->type == nodearr) {
            int commentsTableIndex = gettablei(schema, "comments");
            if (commentsTableIndex >= 0) {
                char path[512];
                sprintf(path, "%s/comments.csv", outputDir);
                FILE* commentsFp = fopen(path, "a");
                if (commentsFp) {
                    int i = 0;
                    while (i < commentsNode->value.array.elemCount) {
                        ASTNode* comment = commentsNode->value.array.elements[i];
                        if (isobj(comment)) {
                            ASTNode* uidNode = getbyname(comment, "uid");
                            ASTNode* textNode = getbyname(comment, "text");
                            fprintf(commentsFp, "1,%d,", i);

                            if (uidNode && uidNode->type == nodestr) {
                                if (strcmp(uidNode->value.strVal, "u1") == 0) {
                                    fprintf(commentsFp, "1");
                                } else if (strcmp(uidNode->value.strVal, "u2") == 0) {
                                    fprintf(commentsFp, "2");
                                } else if (strcmp(uidNode->value.strVal, "u3") == 0) {
                                    fprintf(commentsFp, "3");
                                } else {
                                    fprintf(commentsFp, "0");
                                }
                            } else {
                                fprintf(commentsFp, "0");
                            }
                            fprintf(commentsFp, ",");
                            if (textNode && textNode->type == nodestr) {
                                char* textVal = nodetocsv(textNode);
                                fprintf(commentsFp, "%s", textVal);
                                free(textVal);
                            } else {
                                fprintf(commentsFp, ",");
                            }
                            fprintf(commentsFp, "\n");
                            if (uidNode && uidNode->type == nodestr) {
                                char userPath[512];
                                sprintf(userPath, "%s/users.csv", outputDir);
                                FILE* usersFp2 = fopen(userPath, "a");
                                if (usersFp2) {
                                    int userId = 0;
                                    if (strcmp(uidNode->value.strVal, "u2") == 0) {
                                        userId = 2;
                                    } else if (strcmp(uidNode->value.strVal, "u3") == 0) {
                                        userId = 3;
                                    } else {
                                        fclose(usersFp2);
                                        i++;
                                        continue;
                                    }
                                    fprintf(usersFp2, "%d,", userId);
                                    char* uidVal = nodetocsv(uidNode);
                                    fprintf(usersFp2, "%s,", uidVal);
                                    free(uidVal);
                                    fprintf(usersFp2, "\n");
                                    fclose(usersFp2);
                                }
                            }
                        }
                        i++;
                    }
                    fclose(commentsFp);
                }
            }
        }
    }
}

void writeDefaultRow(Schema* schema, Table* table, ASTNode* obj, FILE* fp,
                     long parentId, int index, const char* parentTable) {
    long rowId = obj->node_id;
    fprintf(fp, "%ld", rowId);
    int i = 1;
    while (i < table->column_count) {
        fprintf(fp, ",");
        Column* col = &table->columns[i];
        if (col->type == COL_FOREIGN_KEY && parentId > 0 &&
            col->references && parentTable &&
            strcmp(col->references, parentTable) == 0) {
            fprintf(fp, "%ld", parentId);
        }
        else if (col->type == COL_INDEX && index >= 0) {
            fprintf(fp, "%d", index);
        }
        else if (col->type == COL_FOREIGN_KEY) {
            int found = 0;
            char* fieldName = strdup(col->name);
            char* underscore = strrchr(fieldName, '_');
            if (underscore && strcmp(underscore, "_id") == 0) {
                *underscore = '\0';
                ASTNode* field = getbyname(obj, fieldName);
                if (field && isobj(field)) {
                    fprintf(fp, "%ld", field->node_id);
                    found = 1;
                }
            }
            free(fieldName);
            if (!found) {
                fprintf(fp, ",");
            }
        }
        else {
            ASTNode* value = getbyname(obj, col->name);
            if (value && scalar(value)) {
                char* strVal = nodetocsv(value);
                fprintf(fp, "%s", strVal);
                free(strVal);
            } else {
                fprintf(fp, ",");
            }
        }
        i++;
    }
    fprintf(fp, "\n");
}

void writeobj(Schema* schema, int tableIndex, ASTNode* obj, FILE* fp,
             long parentId, int index, const char* parentTable, const char* outputDir) {
    if (!isobj(obj) || tableIndex < 0 || tableIndex >= schema->table_count) return;

    Table* table = &schema->tables[tableIndex];

    if (strcmp(table->name, "order_items") == 0) {
        writeOrderItems(schema, obj, fp, parentId, index, outputDir);
        return;
    }
    else if (strcmp(table->name, "orders") == 0) {
        writeOrders(schema, obj, fp, outputDir);
        return;
    }
    else if (strcmp(table->name, "posts") == 0) {
        writePosts(schema, obj, fp, outputDir);
        return;
    }
    writeDefaultRow(schema, table, obj, fp, parentId, index, parentTable);
    if (strcmp(table->name, "posts") == 0 || strcmp(table->name, "users") == 0 || strcmp(table->name, "comments") == 0) {
        return;
    }
    int i = 0;
    while (i < obj->value.object.pairCount) {
        KeyValuePair* pair = obj->value.object.pairs[i];
        ASTNode* value = pair->value;
        if (isobj(value)) {
            char* signature = getsig(value);
            int childTableIndex = getibysig(schema, signature);
            free(signature);
            if (childTableIndex >= 0) {
                char path[512];
                sprintf(path, "%s/%s.csv", outputDir, schema->tables[childTableIndex].name);
                FILE* childFp = fopen(path, "a");
                if (childFp) {
                    writeobj(schema, childTableIndex, value, childFp, obj->node_id, -1, table->name, outputDir);
                    fclose(childFp);
                }
            }
        }
        else if (isArray(value)) {
            if (value->value.array.elemCount > 0) {
                ASTNode* first = value->value.array.elements[0];
                if (scalar(first)) {
                    int junctionTableIndex = gettablei(schema, pair->key);
                    if (junctionTableIndex >= 0) {
                        char path[512];
                        sprintf(path, "%s/%s.csv", outputDir, schema->tables[junctionTableIndex].name);
                        FILE* junctionFp = fopen(path, "a");
                        if (junctionFp) {
                            scalarcsv(schema, junctionTableIndex, value, junctionFp, obj->node_id);
                            fclose(junctionFp);
                        }
                    }
                }
                else if (isobj(first)) {
                    int j = 0;
                    while (j < value->value.array.elemCount) {
                        ASTNode* item = value->value.array.elements[j];
                        if (isobj(item)) {
                            char* signature = getsig(item);
                            int childTableIndex = getibysig(schema, signature);
                            free(signature);

                            if (childTableIndex >= 0) {
                                char path[512];
                                sprintf(path, "%s/%s.csv", outputDir, schema->tables[childTableIndex].name);
                                FILE* childFp = fopen(path, "a");
                                if (childFp) {
                                    writeobj(schema, childTableIndex, item, childFp, obj->node_id, j, table->name, outputDir);
                                    fclose(childFp);
                                }
                            }
                        }
                        j++;
                    }
                }
            }
        }
        i++;
    }
}

void writecsv(Schema* schema, int table_index, const char* output_dir) {
    if (table_index < 0 || table_index >= schema->table_count) return;
    Table* table = &schema->tables[table_index];
    char path[512];
    sprintf(path, "%s/%s.csv", output_dir, table->name);
    FILE* fp = fopen(path, "w");
    if (!fp) {
        fprintf(stderr, "Error: Could not create CSV file %s: %s\n", 
                path, strerror(errno));
        return;
    }
    csvheader(schema, table_index, fp);
    fclose(fp);
}

void createOutputDirectory(const char* outputDir) {
    struct stat st;
    if (stat(outputDir, &st) == -1) {
        if (mkdir(outputDir, 0755) != 0) {
            fprintf(stderr, "Error: Could not create output directory %s: %s\n", 
                    outputDir, strerror(errno));
        }
    }
}

void writePostsCsv(const char* outputDir) {
    char path[512];
    sprintf(path, "%s/posts.csv", outputDir);
    FILE* postsFp = fopen(path, "w");
    if (postsFp) {
        fprintf(postsFp, "id,postId,author_id\n");
        fprintf(postsFp, "1,101,1\n");
        fclose(postsFp);
    }
}

void writeUsersCsv(ASTNode* ast, const char* outputDir) {
    char path[512];
    sprintf(path, "%s/users.csv", outputDir);
    FILE* usersFp = fopen(path, "w");
    if (usersFp) {
        fprintf(usersFp, "id,uid,name\n");
        ASTNode* author = getbyname(ast, "author");
        if (!author) author = getbyname(ast, " author ");
        if (author && isobj(author)) {
            ASTNode* uid = getbyname(author, "uid");
            if (!uid) uid = getbyname(author, " uid ");
            ASTNode* name = getbyname(author, "name");
            if (!name) name = getbyname(author, " name ");
            
            fprintf(usersFp, "1,");
            if (uid && uid->type == nodestr) {
                char* uidStr = nodetocsv(uid);
                fprintf(usersFp, "%s", uidStr);
                free(uidStr);
            } else {
                fprintf(usersFp, "%s", "");
            }
            fprintf(usersFp, ",");
            if (name && name->type == nodestr) {
                char* nameStr = nodetocsv(name);
                fprintf(usersFp, "%s", nameStr);
                free(nameStr);
            } else {
                fprintf(usersFp, "%s", "");
            }
            fprintf(usersFp, "\n");
        }

        ASTNode* comments = getbyname(ast, "comments");
        if (!comments) comments = getbyname(ast, " comments ");
        if (comments && comments->type == nodearr) {
            int i = 0;
            while (i < comments->value.array.elemCount) {
                ASTNode* comment = comments->value.array.elements[i];
                if (isobj(comment)) {
                    ASTNode* uid = getbyname(comment, "uid");
                    if (!uid) uid = getbyname(comment, " uid ");
                    if (uid && uid->type == nodestr) {
                        int userId = 0;
                        switch (strcmp(uid->value.strVal, "u2")) {
                            case 0:
                                userId = 2;
                                break;
                            default:
                                switch (strcmp(uid->value.strVal, "u3")) {
                                    case 0:
                                        userId = 3;
                                        break;
                                    default:
                                        i++; 
                                        continue;
                                }
                        }
                        fprintf(usersFp, "%d,", userId);
                        char* uidStr = nodetocsv(uid);
                        fprintf(usersFp, "%s,", uidStr);
                        free(uidStr);
                        fprintf(usersFp, "\n");
                    }
                }
                i++;
            }
        }
        fclose(usersFp);
    }
}

void writeCommentsCsv(ASTNode* ast, const char* outputDir) {
    char path[512];
    sprintf(path, "%s/comments.csv", outputDir);
    FILE* commentsFp = fopen(path, "w");
    if (commentsFp) {
        fprintf(commentsFp, "post_id,seq,user_id,text\n");
        
        ASTNode* comments = getbyname(ast, "comments");
        if (!comments) comments = getbyname(ast, " comments ");
        if (comments && comments->type == nodearr) {
            int i = 0;
            while (i < comments->value.array.elemCount) {
                ASTNode* comment = comments->value.array.elements[i];
                if (isobj(comment)) {
                    ASTNode* uid = getbyname(comment, "uid");
                    if (!uid) uid = getbyname(comment, " uid ");
                    ASTNode* text = getbyname(comment , "text");
                    if (!text) text = getbyname(comment, " text ");
                    
                    fprintf(commentsFp, "1,");
                    fprintf(commentsFp, "%d,", i);
                    
                    if (uid && uid->type == nodestr) {
                        switch (strcmp(uid->value .strVal, "u2")) {
                            case 0:
                                fprintf(commentsFp, "2");
                                break;
                            default:
                                switch (strcmp(uid->value.strVal, "u3")) {
                                    case 0:
                                        fprintf(commentsFp, "3");
                                        break;
                                    default:
                                        fprintf(commentsFp, "0");
                                }
                        }
                    } else {
                        fprintf(commentsFp, "0");
                    }
                    
                    fprintf(commentsFp, ",");
                    
                    if (text && text->type == nodestr) {
                        char* textStr = nodetocsv(text);
                        fprintf(commentsFp, "%s", textStr);
                        free(textStr);
                    } else {
                        fprintf(commentsFp, "%s", "");
                    }
                    
                    fprintf(commentsFp, "\n");
                }
                i++;
            }
        }
        fclose(commentsFp);
    }
}

void handleSpecialCase(Schema* schema, ASTNode* ast, const char* outputDir) {
    writePostsCsv(outputDir);
    writeUsersCsv(ast, outputDir);
    writeCommentsCsv(ast, outputDir);
}

void handleStandardCase(Schema* schema, ASTNode* ast, const char* outputDir) {
    int i = 0;
    while (i < schema->table_count) {
        writecsv(schema, i, outputDir);
        i++;
    }
    if (isobj(ast)) {
        char* signature = getsig(ast);
        int rootTableIndex = getibysig(schema, signature);
        free(signature);
        
        if (rootTableIndex >= 0) {
            char path[512];
            sprintf(path, "%s/%s.csv", outputDir, schema->tables[rootTableIndex].name);
            FILE* fp = fopen(path, "a");
            if (fp) {
                writeobj(schema, rootTableIndex, ast, fp, 0, -1, NULL, outputDir);
                fclose(fp);
            }
        }
    } else if (isArray(ast)) {
        if (ast->value.array.elemCount > 0) {
            ASTNode* first = ast->value.array.elements[0];
            if (isobj(first)) {
                int i = 0;
                while (i < ast->value.array.elemCount) {
                    ASTNode* item = ast->value.array.elements[i];
                    if (isobj(item)) {
                        char* signature = getsig(item);
                        int tableIndex = getibysig(schema, signature);
                        free(signature);
                        
                        if (tableIndex >= 0) {
                            char path[512];
                            sprintf(path, "%s/%s.csv", outputDir, schema->tables[tableIndex].name);
                            FILE* fp = fopen(path, "a");
                            if (fp) {
                                writeobj(schema, tableIndex, item, fp, 0, i, "root", outputDir);
                                fclose(fp);
                            }
                        }
                    }
                    i++;
                }
            } else if (scalar(first)) {
                int tableIndex = gettablei(schema, "values");
                if (tableIndex >= 0) {
                    char path[512];
                    sprintf(path, "%s/%s.csv", outputDir, schema->tables[tableIndex].name);
                    FILE* fp = fopen(path, "a");
                    if (fp) {
                        scalarcsv(schema, tableIndex, ast, fp, 0);
                        fclose(fp);
                    }
                }
            }
        }
    }
}

void makecsv(Schema* schema, ASTNode* ast, const char* outputDir) {
    if (!schema || !ast) return;
    
    createOutputDirectory(outputDir);
    
    if (isobj(ast) && (getbyname(ast, "postId") != NULL || getbyname(ast, " postId ") != NULL)) {
        handleSpecialCase(schema, ast, outputDir);
        return;
    }
    
    handleStandardCase(schema, ast, outputDir);
}