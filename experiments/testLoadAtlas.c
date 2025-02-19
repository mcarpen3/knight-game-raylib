#include <stdio.h>
#include <raylib.h>
#include <string.h>
#include <stdlib.h>
#include "../datastructs/linkedlist.h"

typedef struct SpriteRect {
    char *filename;
    Rectangle src;
    Rectangle dest;
    float rotation;
} SpriteRect;
static void Destroy(SpriteRect *sr) {
    free(sr->filename);
    free(sr);
}
int main(void) {
    const char *path = "Map1.bin";
    FILE *input = fopen(path, "r");
    if (input == NULL) {
        printf("input was null\n");
        return -1;
    }
    bool done = false;
    List *list = (List *)malloc(sizeof(List));
    list_init(list, (void *)Destroy);
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
        sr->filename = (char*)malloc(nameLength);
        pass = fread(sr->filename, nameLength, 1, input);

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
        list_ins_next(list, NULL, sr);
    }
    ListElmt *li = (ListElmt *)malloc(sizeof(ListElmt));
    li = list->head;
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