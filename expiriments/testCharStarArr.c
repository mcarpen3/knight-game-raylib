#include<stdio.h>
#define ARRAY_LEN(x)(sizeof(x) / sizeof((x)[0]))

int main(void) {
        char *instructions[] = {
        "MOUSESCROLL=zoom", 
        "LEFT click/drag=draw", 
        "'R'=name rect",
        "'ENTER'=save rect",
        "'Q'=save sheet",
        "WASD=pan"
    };
    printf("ARRLEN instructions: %lu", ARRAY_LEN(instructions));
    return 0;
}