#include <stdlib.h>
#include <stdio.h>
#include <time.h>

float rand_float() {
    return (float)rand() / (float)RAND_MAX; 
}

int main(void) {
    srand(time(NULL));
    float randFloat = rand_float() * 100;
    printf("myrandomfloat: %f", randFloat);
}