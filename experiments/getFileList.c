#include <raylib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <linux/limits.h>

int main(void) {
    FilePathList fpl = LoadDirectoryFiles("../spritesheets/background");
    size_t *maxlen = 128l;
    char *fullpath = (char *)malloc(maxlen);
    char *cwd = getcwd(fullpath, 0);
    for (int i = 0; i < fpl.count; ++i) {
        printf("%s\n", fpl.paths[i]);
    }
    return  0;
}