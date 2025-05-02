#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>

/* Check if a directory exists */
int directory_exists(const char* path);

/* Create a directory if it doesn't exist */
int create_directory(const char* path);

/* Get current directory */
char* get_current_directory();

/* Parse command line arguments */
void parse_arguments(int argc, char** argv, int* should_print_ast, char** output_dir);

/* Print usage instructions */
void print_usage(const char* program_name);

/* Check if a string is empty or only whitespace */
int is_empty_string(const char* s);

#endif /* UTIL_H */
