#include <raylib.h>
#include <stdlib.h>
#define _GNU_SOURCE 1
#include <string.h>

struct Sprite {
    char *name;
    Texture2D t;
    float w;
    float h;
    int cnt;
    Rectangle r;
    bool rev;
};

typedef struct Sprite Sprite;

Sprite * SpriteFactory(const char *file, const char *name, float width, float height) {
    // printf("Loading texture...%s, %d, %d, %f\n", file, w, h, scale);
    Sprite *sprite = (Sprite*)malloc(sizeof(Sprite));
    sprite->name = strdup(name);
    sprite->t = LoadTexture(file);
    sprite->cnt = (sprite->t.width / width) * (sprite->t.height / height);
    sprite->r.x = 0.0f;
    sprite->r.y = 0.0f;
    sprite->r.width = sprite->t.width / sprite->cnt;
    sprite->r.height = sprite->t.height;
    sprite->w = width;
    sprite->h = height;
    sprite->rev = false;
    return sprite;
};

void UnloadSprite(Sprite *sprite) {
    UnloadTexture(sprite->t);
    free(sprite->name);
    return free(sprite);
}

void DrawSprite(Sprite *sprite, Rectangle dest) {
    // DrawRectanglePro(dest, (Vector2){0.0f, 0.0f}, 0.0f, BLACK);
    // DrawRectangleLinesEx(dest, 2.0f, RED);
    if (sprite->rev == false) {
        DrawTexturePro(sprite->t, sprite->r, dest, (Vector2){0.0f, 0.0f}, 0.0f, WHITE);        
    } else {
        DrawTexturePro(sprite->t, (Rectangle){
            sprite->r.x, sprite->r.y, -sprite->r.width, sprite->r.height
        }, dest, (Vector2){0.0f, 0.0f}, 0.0f, WHITE);
    }
    // DrawTexturePro(sprite->t, sprite->r, dest, (Vector2){0.0f, 0.0f}, 0.0f, WHITE);
}

void FrameAdvance(Sprite *sprite) {
    sprite->r.x += sprite->w;
    if (sprite->r.x >= sprite->t.width) {
        sprite->r.x = 0.0f;
    }
}