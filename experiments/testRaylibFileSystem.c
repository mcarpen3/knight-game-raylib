#include <raylib.h>
#include <stdio.h>
int main(void) {
    char *ext = GetFileExtension("./fakeFile.c");
    printf("ext: %s\n", ext);
}