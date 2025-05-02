#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "util.h"

/* Check if a directory exists */
int directory_exists(const char* path) {
    struct stat st;
    if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
        return 1;
    }
    return 0;
}

/* Create a directory if it doesn't exist */
int create_directory(const char* path) {
    if (directory_exists(path)) {
        return 1;
    }
    
    if (mkdir(path, 0755) != 0) {
        fprintf(stderr, "Error: Could not create directory %s: %s\n", 
                path, strerror(errno));
        return 0;
    }
    
    return 1;
}

/* Get current directory */
char* get_current_directory() {
    char* buffer = malloc(1024);
    if (!buffer) return NULL;
    
    if (getcwd(buffer, 1024) == NULL) {
        free(buffer);
        return NULL;
    }
    
    return buffer;
}

/* Parse command line arguments */
void parse_arguments(int argc, char** argv, int* should_print_ast, char** output_dir) {
    *should_print_ast = 0;
    *output_dir = NULL;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--print-ast") == 0) {
            *should_print_ast = 1;
        }
        else if (strcmp(argv[i], "--out-dir") == 0 && i + 1 < argc) {
            i++;
            *output_dir = strdup(argv[i]);
        }
        else {
            fprintf(stderr, "Warning: Unknown argument: %s\n", argv[i]);
        }
    }
    
    /* If no output directory specified, use current directory */
    if (*output_dir == NULL) {
        *output_dir = get_current_directory();
    }
}

/* Print usage instructions */
void print_usage(const char* program_name) {
    fprintf(stderr, "Usage: %s [--print-ast] [--out-dir DIR]\n", program_name);
    fprintf(stderr, "  --print-ast   Print the JSON AST to stdout\n");
    fprintf(stderr, "  --out-dir DIR Write CSV files to DIR (default: current directory)\n");
}

/* Check if a string is empty or only whitespace */
int is_empty_string(const char* s) {
    if (!s) return 1;
    
    while (*s) {
        if (!isspace(*s)) {
            return 0;
        }
        s++;
    }
    
    return 1;
}
