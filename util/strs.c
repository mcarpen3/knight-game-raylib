#include <string.h>
#include <stdlib.h>

char **split(const char *str, const char *chr) {
    char *b = strdup(str), *c, **tokens;
    int i = 0;
    c = strtok(b, chr);
    do {
        tokens = realloc(tokens, (i+1) * sizeof(char *));
        tokens[i] = strdup(c);
        c = strtok(NULL, chr);
        i++;
    } while (c != NULL);
    free(b);
    free(c);
    return tokens;
}

