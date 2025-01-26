#include <stdio.h>

static void displayObjBounds(const char* filename) {
    struct rec {
        float x;
        float y;
        float w;
        float h;
    };
    typedef struct rec rec;
    struct params {
        struct rec d;
        float     s;
    } params;
    FILE* file = fopen(filename, "r");
    fread(&params, sizeof params, 1, file);
    fclose(file);
    printf("%s: %.3f, [%.3f, %.3f, %.3f, %.3f]\n", filename, params.s, params.d.x, params.d.y, params.d.w, params.d.h);

}

int main(void) {
    struct mys {
        float f;
        int   i;
    } mys;
    printf("sizeof mystruct = %lu\n", sizeof mys);
    displayObjBounds("../resources/skeleton-params.bin");
    displayObjBounds("../resources/knight-params.bin");
}

