#include "raylib.h"
#include <stdio.h>


int main(void) {
    Vector2 m = (Vector2){2.0f, 2.0f};
    m.x *= 2.0f;
    printf("x: %f, y: %f", m.x, m.y);
};