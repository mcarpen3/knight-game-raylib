#ifndef MAPOBJECT_H
#define MAPOBJECT_H

#include <raylib.h>
#include <raymath.h>
#include "../Datastructs/linkedlist.h"
#include "../Datastructs/doublelinkedlist.h"
#include "../Datastructs/set.h"

#define MAX_VELOCITY 10.0f
#define ACCEL 0.4f
#define JMP_ACCL -3.0f
#define MAX_ROTATE 359.0f
#define ROTATE_ACCEL 0.2f
#define MAX_GRAVITY 14.0f
// #define MAX_GRAVITY     14.0f
#define MAX_JMP_VEL -9.0f
#define MAX_JMP_CNT 15

typedef enum objtype_ {
    Player, Enemy, Item, Projectile, Env
} objtype;

typedef enum acttype_ {
    NoAction, Attack, Attack2, AttackNoMovement, AttackCombo, 
    AttackComboNoMovement, Crouch, CrouchAttack, CrouchFull, 
    CrouchTransition, CrouchWalk, Dash, Death, DeathNoMovement,
    Fall, Hit, Idle, Jump, JumpFallInBetween, Roll,
    Run, Slide, SlideFull, SlideTransitionEnd,
    SlideTransitionStart, TurnAround, WallClimb,
    WallClimbNoMovement, WallHang, WallSlide
} acttype;

typedef struct ObjVelLims {
    float xmin;
    float xmax;
    float ymin;
    float ymax;
} ObjVelLims;

typedef struct Object_ {
    char *name;
    int type;
    float scale;
    float rotation;
    Vector2 position;
    DList *sprites;
    DListElmt *curSpriteEl;
    Vector2 vel;
    ObjVelLims ov;
    bool attack;
    bool reverse;
    bool alive;
    int jmpcnt;
} Object;

void DestroyObject(Object *o);

void UpdateMapObject(Object *o);

void SetAction(Object *o, int action);

void DrawMapObject(Object *o);

#endif // MAPOBJECT_H