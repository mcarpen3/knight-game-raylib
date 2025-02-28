#ifndef MAPOBJECT_H
#define MAPOBJECT_H

#include <raylib.h>
#include "../datastructs/doublelinkedlist.h"
#include "../datastructs/linkedlist.h"

typedef enum objtype_ {
    Player, Enemy, Item, Projectile, Env
} objtype;

typedef struct Object_ {
    char *name;
    objtype type;
    float scale;
    float rotation;
    Vector2 position;
    DList *sprites;
    DListElmt *curSpriteEl;
} Object;

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

#endif // MAPOBJECT_H