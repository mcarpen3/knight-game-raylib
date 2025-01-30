#include <stdio.h>
int main(void) {
    int ten = 10;
    float five = 5.0f;
    printf("%f", ten / five);
    printf("sizeof: { double: %lu, float: %lu }\n", sizeof(double), sizeof(float));
}