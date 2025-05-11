#ifndef HELPER_H
#define HELPER_H

#include <stdio.h>

int direxists(const char* p);
int createdir(const char* p);
char* getcurrdir();
void parseargs(int argc, char** argv, int* printast, char** outdir);
int isempty(const char* s);

#endif /* HELPER_H */
