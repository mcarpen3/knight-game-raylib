#include "../getSpriteSheets.c"
#include <stdlib.h>
int main(void) {
    const char *p = "/home/matt/Documents/c/raylib/knight/Colour1/NoOutline/120x80_PNGSheets";
    size_t *count = (size_t *)malloc(sizeof(size_t));
    char **sheets = getSpriteSheets(p, count);
    printSpriteSheets(sheets, count);
}