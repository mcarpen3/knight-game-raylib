#include "TextureSource.h"

int matchTexture(const void *key1, const void *key2) {
    char *file1 = ((TextureSource *)key1)->filename;
    char *file2 = ((TextureSource *)key2)->filename;
    return strcmp(file1, file2) == 0 ? 1 : 0;
}

void DestroyTextureSource(TextureSource *t) {
    UnloadTexture(t->texture);
    free(t->filename);
}