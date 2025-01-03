#include <raylib.h>
#include <stdlib.h>
#define _GNU_SOURCE 1
#include <string.h>

struct Sprite {
    Texture2D t;
    int w;
    int h;
    int cnt;
    Rectangle r;
    float s;
    bool rev;
};

typedef struct Sprite Sprite;

static Rectangle GetScaledDest(Sprite *sprite, Rectangle dest);

Sprite * SpriteFactory(const char *file, int w, int h, float scale) {
    Sprite *sprite = (Sprite*)malloc(sizeof(Sprite));
    sprite->t = LoadTexture(file);
    sprite->w = w;
    sprite->h = h;
    sprite->cnt = sprite->t.width / w * sprite->t.height / h;
    sprite->r.x = 0.0f;
    sprite->r.y = 0.0f;
    sprite->r.width = w;
    sprite->r.height = h;
    sprite->s = scale;
    sprite->rev = false;
    return sprite;
};

void UnloadSprite(Sprite *sprite) {
    UnloadTexture(sprite->t);
    return free(sprite);
}

void DrawSprite(Sprite *sprite, Rectangle dest) {
    Rectangle scaledDest = GetScaledDest(sprite, dest);
    // DrawRectanglePro(scaledDest, (Vector2){-dest.width / 2, dest.height /2}, 0.0f, BLACK);
    // DrawRectangleLinesEx(dest, 2.0f, RED);
    if (sprite->rev == false) {
        DrawTexturePro(sprite->t, sprite->r, scaledDest, (Vector2){-dest.width / 2, dest.height / 2}, 0.0f, WHITE);        
    } else {
        DrawTexturePro(sprite->t, (Rectangle){
            sprite->r.x, sprite->r.y, -sprite->r.width, sprite->r.height
        }, scaledDest, (Vector2){-dest.width / 2, dest.height / 2}, 0.0f, WHITE);
    }
}

void FrameAdvance(Sprite *sprite) {
    sprite->r.x += sprite->w;
    if (sprite->r.x >= sprite->t.width) {
        sprite->r.x = 0;
    }
}

static Rectangle GetScaledDest(Sprite *sprite, Rectangle dest) {
    return (Rectangle){
        dest.x - dest.width * sprite->s / 2,
        dest.y - dest.height * sprite->s / 2,
        dest.width * sprite->s,
        dest.height * sprite->s
    };
}