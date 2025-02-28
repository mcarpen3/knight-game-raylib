#include <raylib.h>
#include "../datastructs/linkedlist.h"
#include "../sprite/sprite.h"
#include <stdio.h>

static void LoadSpriteList(List *sprites) {
    const char *path = "/home/matt/Documents/c/raylib/experiments/Map1.bin";
    FILE *input = fopen(path, "r");
    if (input == NULL) {
        printf("ERROR: input was null\n");
        return;
    }
    bool done = false;
    while (!done) {
        SpriteRect *sr = (SpriteRect *)malloc(sizeof(SpriteRect));
        size_t nameLength;
        size_t pass = 0;
        pass = fread(&nameLength, sizeof(size_t), 1, input);
        if (pass != 1) {
            done = true;
            continue;
        }
        printf("nameLength: %lu\n", nameLength);
        sr->filename = (char*)malloc(nameLength + 1);
        pass = fread(sr->filename, nameLength, 1, input);
        printf("last char of sr-filename '%c'\n", sr->filename[strlen(sr->filename)]);
        sr->filename[nameLength] = '\0';
        printf("last char of sr-filename '%c'\n", sr->filename[strlen(sr->filename)]);

        pass = fread(&sr->src, sizeof(Rectangle), 1, input);
        if (pass != 1) {
            done = true;
            continue;
        }
        pass = fread(&sr->dest, sizeof(Rectangle), 1, input);
        if (pass != 1) {
            done = true;
            continue;
        }
        pass = fread(&sr->rotation, sizeof(float), 1, input);
        sr->texture = LoadTexture(sr->filename);
        list_ins_next(sprites, NULL, sr);
    }
    ListElmt *li = (ListElmt *)malloc(sizeof(ListElmt));
    li = sprites->head;
    while(li != NULL) {
        SpriteRect *sr = (SpriteRect *)li->data;
        printf("%s,\nsrc:{%.3f, %.3f, %.3f, %.3f},\ndest:{%.3f, %.3f, %.3f, %.3f},\nrot: %f\n\n", 
            sr->filename,
            sr->src.x, sr->src.y, sr->src.width, sr->src.height,
            sr->dest.x, sr->dest.y, sr->dest.width, sr->dest.height,
            sr->rotation);
        li = li->next;
    }
}

void LoadWorld(List *sprites, List *colliderLines, List *colliderRects) {
    printf("INFO: Loading World...\n");
    // List *colliderLineList = (List *)malloc(sizeof(List));
    // List *colliderRectList = (List *)malloc(sizeof(List));
    LoadSpriteList(sprites);
}