#include <raylib.h>
#include "../Datastructs/set.h"
#include <string.h>

typedef struct TextureSource_ {
    Texture2D texture;
    char *filename;
} TextureSource;

int matchTexture(const void *key1, const void *key2);
void DestroyTextureSource(TextureSource *t);

