#include<stdio.h>
#include "../util/basename.c"

int main(void) {
    char *path = "/home/matt/Documents/test.c";
    char **tokens = basename(path);
    int i = 0;
    while(tokens[i] != NULL) {
        printf("%d.%30s\n", i, tokens[i]);
        i++;
    }
    return 0;
}