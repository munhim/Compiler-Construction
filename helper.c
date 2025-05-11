#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "helper.h"


int direxists(const char* p) {
    struct stat st;
    return (stat(p, &st) == 0 && S_ISDIR(st.st_mode)) ? 1 : 0;
}


int createdir(const char* p) {
    if (!direxists(p)) {
        if (mkdir(p, 0755) != 0) {
            fprintf(stderr, "Error: Failed to create dir '%s': %s\n", p, strerror(errno));
            return 0;
        }
    }
    return 1;
}

char* getcurrdir() {
    size_t size = 1024;
    char* buf = malloc(size);
    if (buf == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return NULL;
    }
    if (getcwd(buf, size) == NULL) {
        perror("Error getting current directory");
        free(buf);
        return NULL;
    }
    return buf;
}

void parseargs(int argc, char** argv, int* printast, char** outdir) {
    *printast = 0;
    *outdir = NULL;

    int i = 1;
    while (i < argc) {
        if (!strcmp(argv[i], "--print-ast")) {
            *printast = 1;
        } 
        else if (!strcmp(argv[i], "--out-dir") && i + 1 < argc) {
            *outdir = strdup(argv[++i]);
            if (*outdir && !direxists(*outdir) && !createdir(*outdir)) {
                fprintf(stderr, "Error: Can't create dir %s\n", *outdir);
                free(*outdir); *outdir = NULL;
            }
        } 
        else if (strcmp(argv[i], "--help") && strcmp(argv[i], "-h")) {
            fprintf(stderr, "Unknown arg: %s\n", argv[i]);
        }
        i++;
    }

    if (!*outdir) *outdir = getcurrdir();
}

int isempty(const char* s) {
    if (!s) return 1;
    while (*s && isspace(*s)) s++;
    return !*s;
}

