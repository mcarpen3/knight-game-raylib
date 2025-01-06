#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "../sprite/sprite.h"
#include "../resources/getSpriteFiles.c"
#include "../util/path.c"
#include "actions.h"

#define MAX_VELOCITY    10.0f
#define ACCEL           0.4f
#define JMP_ACCL        3.0f
#define MAX_ROTATE      359.0f
#define ROTATE_ACCEL    0.2f
#define MAX_GRAVITY     14.0f
// #define MAX_GRAVITY     14.0f
#define MAX_JMP_VEL     9.0f
#define MAX_JMP_CNT     15

#define SCALAR          0.1f
#define MAX_SCALE       3.0f
#define MIN_SCALE       1.0f

#define OBJECT_SCALE    3.0f
#define WIDTH           120
#define HEIGHT          80
#define RADIUS          30.0f

#define SPRITE_ROOT "/home/matt/Documents/c/raylib/spritesheets"

static float Clamp(float value, float min, float max);

struct Object {
    char *name;
    Vector2 pos;
    float rad;
    Vector2 vel;
    float rot;
    int fc;
    float rotVel;
    float g;
    Sprite *sprite;
    Rectangle c;
    float scale;
    float w;
    float h;
    unsigned char jc;
    Sprite **sprites;
    size_t *spriteCnt;
    char *act;
};

typedef struct Object Object;

static void LoadSprites(Object *obj);
static void SetSprite(Object *obj);


Object *ObjectFactory(const char *name, Vector2 pos) {
    Object *o = (Object *)malloc(sizeof(Object));
    o->name = strdup(name);
    o->act = strdup(IDLE);
    o->pos.x = pos.x;
    o->pos.y = pos.y;
    o->scale = OBJECT_SCALE;
    o->vel.x = 0.0f;
    o->vel.y = MAX_GRAVITY;
    o->fc = 0;
    o->rad = RADIUS;
    o->rot = 0.0f;
    o->rotVel = 0.0f;
    o->w = WIDTH;
    o->h = HEIGHT;
    o->c = (Rectangle){
        pos.x - o->scale * o->w / 2,
        pos.y - o->h * o->scale / 2,
        o->w,
        o->h
    };
    o->jc = 0;
    o->sprites = NULL;
    LoadSprites(o);
    o->sprite = o->sprites[0];
    SetSprite(o);

    return o;
}

void SetPosition(Object *obj, Vector2 pos) {
    obj->pos.x = pos.x;
    obj->pos.y = pos.y;
}
void Update(Object *obj) {
    obj->fc++;
    obj->vel.y = Clamp(obj->vel.y + ACCEL, -MAX_JMP_VEL, MAX_GRAVITY);
    obj->pos.x = Clamp(obj->pos.x + obj->vel.x, obj->w/2, GetScreenWidth() - obj->w / 2);
    obj->pos.y = Clamp(obj->pos.y + obj->vel.y, obj->h/2, GetScreenHeight() - obj->h / 2);
    obj->c.x = obj->pos.x - obj->w/2;
    obj->c.y = obj->pos.y - obj->h/2;
    if (obj->fc >= (60 / obj->sprite->cnt)) {
        obj->fc = 0;
        FrameAdvance(obj->sprite);
    }
    if (obj->vel.x < 0) {
        obj->sprite->rev = true;
    } else if (obj->vel.x > 0) {
        obj->sprite->rev = false;
    }
    //SetSprite(obj);
}
void Draw(Object *obj) {
    // DrawRectangleLinesEx(obj->c, 1.0f, WHITE);
    // DrawCircle(obj->pos.x, obj->pos.y, obj->rad, WHITE);
    DrawSprite(obj->sprite, obj->c);
}
void Forward(Object *obj) {
    obj->vel.x = Clamp(obj->vel.x + ACCEL, -MAX_VELOCITY, MAX_VELOCITY);
}
void Backward(Object *obj) {
    obj->vel.x = Clamp(obj->vel.x - ACCEL, -MAX_VELOCITY, MAX_VELOCITY);
}
void Decel(Object *obj) {
    if (obj->vel.x > 0) {
        obj->vel.x = Clamp(obj->vel.x - ACCEL, 0.0f, MAX_VELOCITY);
    } else {
        obj->vel.x = Clamp(obj->vel.x + ACCEL, -MAX_VELOCITY, 0.0f);
    }
}
void Jump(Object *obj) {
    if (obj->jc >= MAX_JMP_CNT) return;
    obj->jc++;
    obj->vel.y = Clamp(obj->vel.y - JMP_ACCL, -MAX_JMP_VEL, 0.0f);
}
void Land(Object *obj) {
    obj->jc = MAX_JMP_CNT;
    if (obj->pos.y + obj->h / 2 < GetScreenHeight()) return;
    obj->jc = 0;
}

static float Clamp(float value, float min, float max) {
    if (value > max) return max;
    if (value < min) return min;
    return value;
}
void Destroy(Object *obj) {
    for (int i = 0; i < *obj->spriteCnt; ++i) {
        UnloadSprite(obj->sprites[i]);
    }
    free(obj->spriteCnt);
    free(obj->name);
    free(obj->act);
    free(obj);
    printf("INFO: Object freed successfully!\n");
}

void SetSprite(Object *obj) {
    char *act;
    if (obj->vel.y < 0) {
        act = strdup(JUMP);
    } else if (obj->vel.x != 0) {
        act = strdup(RUN);
    } else {
        act = strdup(IDLE);
    }
    if (0 == strcmp(obj->act, act)) {
        free(act);
        return;
    }
    for (int i = 0; i < *obj->spriteCnt; ++i) {
        if (0 == strcmp(obj->sprites[i]->name, act)) {
            obj->sprite = obj->sprites[i];
            obj->act = strdup(act);
            free(act);
            return;
        }
    }
    obj->sprite = obj->sprites[0];
    free(obj->act);
    obj->act = strdup(act);
    free(act);
}

static void LoadSprites(Object *obj) {
    //printf("%s/%s %ld, %ld\n", SPRITE_ROOT, obj->name, strlen(SPRITE_ROOT), strlen(obj->name));
    char *subdir = (char *)malloc(strlen(SPRITE_ROOT) + strlen(obj->name) + 2);
    sprintf(subdir, "%s/%s", SPRITE_ROOT, obj->name);
    obj->spriteCnt = (size_t *)malloc(sizeof(size_t));
    char **sheets = getSpriteSheets(subdir, obj->spriteCnt);
    free(subdir);
    printf("INFO: OBJECT: LoadSprites: spriteCnt: %lu\n", *obj->spriteCnt);
    for (int i = 0; i < *obj->spriteCnt; ++i) {
        obj->sprites = realloc(obj->sprites, (i + 1) * sizeof(Sprite *));
        char *basefn = basename(sheets[i]);
        obj->sprites[i] = SpriteFactory(sheets[i], obj->w, obj->h, obj->scale, basefn);
        free(sheets[i]);
        free(basefn);
    }
}

// void Grow(Object *obj) {
//     obj->sprite->s += SCALAR;
//     if (obj->sprite->s >= MAX_SCALE) {
//         obj->sprite->s = MAX_SCALE;
//     }
// }
// void Shrink(Object *obj) {
//     obj->sprite->s -= SCALAR;
//     if (obj->sprite->s < MIN_SCALE) {
//         obj->sprite->s = MIN_SCALE;
//     }
// }