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
        else if (strcmp(argv[i], "--out-dir") == 0) {
            if (i + 1 < argc) {
                i++;
                *output_dir = strdup(argv[i]);
                
                /* Make sure the directory exists or can be created */
                if (!directory_exists(*output_dir)) {
                    if (!create_directory(*output_dir)) {
                        fprintf(stderr, "Error: Could not create output directory: %s\n", *output_dir);
                        free(*output_dir);
                        *output_dir = NULL;
                    }
                }
            } else {
                fprintf(stderr, "Error: --out-dir requires a directory path\n");
                print_usage(argv[0]);
            }
        }
        else if (strcmp(argv[i], "--help") != 0 && strcmp(argv[i], "-h") != 0) {
            /* Ignore --help/-h as they're handled in main */
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
    fprintf(stderr, "Usage: %s < input.json [--print-ast] [--out-dir DIR]\n\n", program_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  --help, -h    Show this help message and exit\n");
    fprintf(stderr, "  --print-ast   Print the JSON AST to stdout before generating CSV\n");
    fprintf(stderr, "  --out-dir DIR Write CSV files to DIR (default: current directory)\n\n");
    fprintf(stderr, "Description:\n");
    fprintf(stderr, "  This program converts JSON input into relational CSV files.\n");
    fprintf(stderr, "  It takes JSON from standard input and creates CSV files based on\n");
    fprintf(stderr, "  the JSON structure with appropriate primary/foreign key relationships.\n");
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
