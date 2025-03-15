#ifndef SPRITE_H
#define SPRITE_H
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../Datastructs/linkedlist.h"

typedef struct SpriteRect
{
    char *name;
    char *filename;
    int action;
    Rectangle r;
    Texture2D texture;
    int frameCount;
    int frameIdx;
    int frameCounter;
    List *colliders;
    ListElmt *curCldrEl;
} SpriteRect;

typedef struct Line_ {
    Vector2 p1;
    Vector2 p2;
} Line;

typedef enum cldrtype_ {
    RectType,
    LineType,
} cldrtype;

typedef struct Collider_ {
    cldrtype type;
    union {
        Rectangle rect;
        Line line;
    } data;
} Collider;

void UpdateSprite(SpriteRect *sprite);

void DestroySprite(SpriteRect *sprite);

void DestroyCollider(Collider *collider);

void DrawSprite(SpriteRect *sprite, Vector2 position, float scale, float rotation, bool reverse);

#endif // SPRITE_H