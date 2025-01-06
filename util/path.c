#include <string.h>
#include <stdlib.h>
#include "strs.c"

char **parse(const char *path) {
    return split(path, "/");
}

char *basename(const char *path) {
    char *b = strdup(path), *c, *token;
    c = strtok(b, "/");
    do {
        token = strdup(c);
        c = strtok(NULL, "/");
    } while(c != NULL);
    free(b);
    free(c);
    return token;
}