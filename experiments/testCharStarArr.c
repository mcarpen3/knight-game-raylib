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
    char *inst2[][2] = {
        {"hello", "world"},
        {"world"},
    };
    printf("ARRLEN instructions: %lu\n", ARRAY_LEN(instructions));
    printf("ARRLEN inst2: %lu\n", ARRAY_LEN(inst2[0]));
    for (int i = 0; i < ARRAY_LEN(inst2); ++i) {
        printf("ARRAY_LEN inst2[%d] = %lu\n", i, ARRAY_LEN(inst2[i]));
        for (int j = 0; j < ARRAY_LEN(inst2[i]); ++j) {
            if (inst2[i][j] != NULL)
                printf("%s\n", inst2[i][j]);
        }
    }
    return 0;
}