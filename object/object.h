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
#define MAX_SCALE       10.0f
#define MIN_SCALE       0.5f

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
    Rectangle d;
    float offsetX;
    Vector2 dscale;
    float spriteScale;
    Vector2 originDims;
    unsigned char jc;
    Sprite **sprites;
    size_t *spriteCnt;
    char *act;
    bool attack;
    bool reverse;
};

typedef struct Object Object;

static void LoadSprites(Object *obj);
static void SetSprite(Object *obj);

Object *ObjectFactory(const char *name, Vector2 pos, Vector2 origDims) {
    Object *o = (Object *)malloc(sizeof(Object));
    o->name = strdup(name);
    o->act = strdup(IDLE);
    o->pos.x = pos.x;
    o->pos.y = pos.y;
    o->spriteScale = 1.0f;
    o->vel.x = 0.0f;
    o->vel.y = MAX_GRAVITY;
    o->fc = 0;
    o->rad = RADIUS;
    o->rot = 0.0f;
    o->rotVel = 0.0f;
    o->originDims = origDims;
    o->dscale = (Vector2){1.0f, 2.0f};
    o->c = (Rectangle){
        pos.x - o->originDims.x / 2,
        pos.y - o->originDims.y / 2,
        o->originDims.x,
        o->originDims.y
    };
    o->d = (Rectangle) {
        pos.x - o->originDims.x * o->dscale.x / 2,
        pos.y - o->originDims.y * o->dscale.y,
        o->originDims.x * o->dscale.x,
        o->originDims.y * o->dscale.y
    };
    o->jc = 0;
    o->sprites = NULL;
    o->sprite = NULL;
    o->attack = false;
    o->offsetX = 0.0f;
    LoadSprites(o);
    SetSprite(o);
    o->reverse = false;
    return o;
}

void SetPosition(Object *obj, Vector2 pos) {
    obj->pos.x = pos.x;
    obj->pos.y = pos.y;
}

void Update(Object *obj) {
    float xmax = GetScreenWidth() - obj->originDims.x / 2;
    float ymax = GetScreenHeight() - obj->originDims.y / 2;
    obj->vel.y = Clamp(obj->vel.y + ACCEL, -MAX_JMP_VEL, MAX_GRAVITY);
    obj->pos.x = Clamp(obj->pos.x + obj->vel.x, obj->originDims.x / 2, xmax);
    obj->pos.y = Clamp(obj->pos.y + obj->vel.y, obj->originDims.y / 2, ymax);
    obj->c.x = obj->pos.x - (obj->originDims.x / 2);
    obj->c.y = obj->pos.y - (obj->originDims.y / 2);
    obj->d.x = obj->pos.x - (obj->originDims.x * obj->dscale.x / 2) + obj->offsetX;
    obj->d.y = obj->pos.y - (obj->originDims.y * obj->dscale.y) + (obj->originDims.y / 2);
    obj->d.height = obj->originDims.y * obj->dscale.y;
    obj->d.width = obj->originDims.x * obj->dscale.x;
    obj->fc++;
    if (obj->fc >= (60 / (obj->sprite->cnt))) { // add a scale to this and factor in the elapsed time
        obj->fc = 0;
        FrameAdvance(obj->sprite);
    }
    if (obj->vel.x < 0) {
        obj->reverse = true;
    } else if (obj->vel.x > 0) {
        obj->reverse = false;
    }
    SetSprite(obj);
    obj->sprite->rev = obj->reverse;
}

void Draw(Object *obj) {
    DrawRectangleLinesEx(obj->c, 1.0f, WHITE);
    DrawRectangleLinesEx(obj->d, 1.0f, GREEN);
    // DrawCircle(obj->pos.x, obj->pos.y, obj->rad, WHITE);
    DrawSprite(obj->sprite, (Rectangle){
        obj->pos.x - obj->originDims.x * obj->spriteScale / 2,
        obj->pos.y - obj->originDims.y * obj->spriteScale + (obj->originDims.y / 2),
        obj->originDims.x * obj->spriteScale,
        obj->originDims.y * obj->spriteScale
    });
}

void Forward(Object *obj) {
    obj->vel.x = Clamp(obj->vel.x + ACCEL, -MAX_VELOCITY, MAX_VELOCITY);
}

void Backward(Object *obj) {
    obj->vel.x = Clamp(obj->vel.x - ACCEL, -MAX_VELOCITY, MAX_VELOCITY);
}

void Jump(Object *obj) {
    if (obj->jc >= MAX_JMP_CNT) return;
    obj->jc++;
    obj->vel.y = Clamp(obj->vel.y - JMP_ACCL, -MAX_JMP_VEL, 0.0f);
}

void Attack(Object *obj) {
    obj->attack = true;
}

void Decel(Object *obj) {
    if (obj->vel.x > 0) {
        obj->vel.x = Clamp(obj->vel.x - ACCEL, 0.0f, MAX_VELOCITY);
    } else {
        obj->vel.x = Clamp(obj->vel.x + ACCEL, -MAX_VELOCITY, 0.0f);
    }
}

void Rest(Object *obj) {
    obj->attack = false;
    if (obj->pos.y + obj->c.height / 2 < GetScreenHeight()) {
        obj->jc = MAX_JMP_CNT;
    } else {
        obj->jc = 0;
    }
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

void ScaleSprite(Object *obj) {
    if (IsKeyDown(KEY_UP)) {
        obj->spriteScale = Clamp(obj->spriteScale + SCALAR, MIN_SCALE, MAX_SCALE);
    } else if (IsKeyDown(KEY_DOWN)){
        obj->spriteScale = Clamp(obj->spriteScale - SCALAR, MIN_SCALE, MAX_SCALE);
    }
}

void ScaleCollider(Object *obj) {
    if (IsKeyDown(KEY_UP)) {
        obj->dscale.y = Clamp(obj->dscale.y + SCALAR, MIN_SCALE, MAX_SCALE);
    } else if (IsKeyDown(KEY_DOWN)){
        obj->dscale.y = Clamp(obj->dscale.y - SCALAR, 2.0f, MAX_SCALE);
    } else if (IsKeyDown(KEY_RIGHT)) {
        obj->dscale.x = Clamp(obj->dscale.x + SCALAR, MIN_SCALE, MAX_SCALE);
    } else if (IsKeyDown(KEY_LEFT)) {
        obj->dscale.x = Clamp(obj->dscale.x - SCALAR, MIN_SCALE, MAX_SCALE);
    }
}

void ShiftOffset(Object *obj) {
    if (IsKeyDown(KEY_RIGHT)) {
        obj->offsetX = Clamp(obj->offsetX + SCALAR, -MAX_SCALE, MAX_SCALE);
    } else if (IsKeyDown(KEY_LEFT)) {
        obj->offsetX = Clamp(obj->offsetX - SCALAR, -MAX_SCALE, MAX_SCALE);
    }
}

static float Clamp(float value, float min, float max) {
    if (value > max) return max;
    if (value < min) return min;
    return value;
}

static void SetSprite(Object *obj) {
    char *act;
    if (obj->attack) {
        act = strdup(ATTACK);
    } else if (obj->vel.y < 0) {
        act = strdup(JUMP);
    } else if (fabs(obj->vel.x) < 5.0f && 
        ((obj->vel.x < 0 && IsKeyDown(KEY_RIGHT)) || 
         (obj->vel.x > 0 && IsKeyDown(KEY_LEFT)))) {
            act = strdup(TURNAROUND);
    } else if (obj->vel.x != 0) {
        act = strdup(RUN);
    } else {
        act = strdup(IDLE);
    }
    if (0 == strcmp(obj->act, act) && obj->sprite != NULL) {
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
    for (int i = 0; i < *obj->spriteCnt; ++i) {
        obj->sprites = realloc(obj->sprites, (i + 1) * sizeof(Sprite *));
        char *basefn = basename(sheets[i]);
        obj->sprites[i] = SpriteFactory(sheets[i], basefn, obj->originDims.x, obj->originDims.y);
        free(sheets[i]);
        free(basefn);
    }
}
