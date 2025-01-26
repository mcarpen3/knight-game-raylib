#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>

int main(void) {
    srand(time(NULL));
    float r = rand() / (float)RAND_MAX;
    printf("rand: %f", r);
    const int w = 1920;
    const int h = 1080; 
}