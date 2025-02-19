#include<raylib.h>
#include<stdio.h>

int main(void) {
    printf("raylib GetDirectoryFiles test\n");

    FilePathList pngs = LoadDirectoryFilesEx("/home/matt/Documents/c/raylib/spritesheets", ".png", true);
    printf("found %d files!\n", pngs.count);
    for (int i = 0; i < pngs.count; ++i) {
        printf("%3d. %s\n", i, pngs.paths[i]);
    }
}