#include<stdlib.h>
#include<stdio.h>

typedef struct {
    char *mystring;
} mystruct;

int main(void) {
    mystruct *test1 = (mystruct*)malloc(sizeof(mystruct));
    mystruct *test2;
    printf("test1 is null? %s\n", test1 == NULL ? "true" : "false");
    printf("test2 is null? %s\n", test2 == NULL ? "true" : "false");
    printf("test1->mystring is null? %s\n", test1->mystring == NULL ? "true":"false");
}