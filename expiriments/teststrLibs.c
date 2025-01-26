#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define STRING_LITERAL "STRINGLITERAL"
static char *getString(char **out);
int main(void) {
    int v = -1;
    char vc = '\0';
    if (vc) {
        printf("value is truthy\n");
    } else {
        printf("value is falsy\n");
    }
    return 0;
    typedef struct test {
        char *name;
    } test;
    test *a = (test *)malloc(sizeof(test));
    a->name = "_Jump.png";
    const char *basefn0 = "_Jump.png";
    char *basefn1 = strdup("Jump.png");
    printf("a.name: %s %p, basefn1: %s %p, are %s\n", a->name, a->name, basefn1, basefn1, *(a->name) == *basefn1 ? "equal":"not equal");
    // free(basefn0);
    free(a);
    free(basefn1);
    return 0;
    char *in;
    getString(&in);
    printf("the value of in = %s\n", in);
    return 0;
    printf("strlen of string literal %ld\n", strlen(STRING_LITERAL));
    char *tmp0 = "hello";
    char *copy = (char *)malloc(strlen(STRING_LITERAL) + strlen(tmp0) + 2);
    sprintf(copy, "%s/%s", STRING_LITERAL, tmp0);
    printf("%s\n", copy);
    char tmp1[] = {'w','o','r','l','d', '\0'};
    printf("strlen tmp0 %ld, strlen tmp1 %ld\n", strlen(tmp0), strlen(tmp1));
    printf("sizeof tmp0 %ld, sizeof tmp1 %ld\n", sizeof(tmp0), sizeof(tmp1));
    printf("sizeof char %ld, sizeof char* %ld\n", sizeof(char), sizeof(char *));
    return 0;
}
static char *getString(char **out) {
    *out = strdup("wassup, doc?");
    return *out;
}