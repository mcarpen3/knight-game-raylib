#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

char **getSpriteSheets(const char *path, size_t *count) {
    DIR *dir;
    struct dirent *entry;
    char **paths = NULL;
    size_t dirLen = strlen(path);
    *count = 0;
    // Open the directory
    dir = opendir(path); 
    if (dir == NULL) {
        perror("opendir");
        return NULL;
    }
    // Read each entry in the directory
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == 8) {
            paths = realloc(paths, (*count + 1) * sizeof(char*));
            paths[*count] = (char *)malloc(dirLen + strlen(entry->d_name)+2);
            sprintf(paths[*count], "%s/%s", path, entry->d_name);
            (*count)++;
        }
    }

    // Close the directory
    closedir(dir);

    return paths;
}

void printSpriteSheets(char **paths, size_t *count) {
    if (paths == NULL) return;
    for (int i = 0; i < *count; ++i) {
        printf("%d.\t%s\n", i + 1, paths[i]);
    }
}