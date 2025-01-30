#include <stdio.h>
#include <raylib.h>
#include <string.h>
#include <stdlib.h>

typedef struct SpriteRect {
    char *name;
    Rectangle r;
} SpriteRect;
int main(void) {
    const char *path = "../spritesheets/background/terrain.atlas";
    FILE *input = fopen(path, "r");
    if (input == NULL) {
        printf("input was null\n");
        return -1;
    }
    SpriteRect *sr = (SpriteRect *)malloc(sizeof(SpriteRect));
    size_t nameLength;
    fread(&nameLength, sizeof(size_t), 1, input);
    printf("nameLength: %lu\n", nameLength);
    sr->name = (char*)malloc(nameLength);
    fread(sr->name, nameLength, 1, input);
    fread(&sr->r, sizeof(Rectangle), 1, input);
    printf("%s, %.3f, %.3f, %.3f, %.3f\n", sr->name, sr->r.x, sr->r.y, sr->r.width, sr->r.height);
}