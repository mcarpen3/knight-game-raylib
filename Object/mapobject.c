#include "mapobject.h"
#include "../Sprite/sprite.h"

void DestroyObject(Object *o)
{
    dlist_destroy(o->sprites);
    free(o->name);
    free(o);
};

void UpdateMapObject(Object *obj)
{
    if (IsKeyDown(KEY_RIGHT))
    {
        obj->vel.x = Clamp(obj->vel.x + ACCEL, 0, MAX_VELOCITY);
    }
    else if (IsKeyDown(KEY_LEFT))
    {
        obj->vel.x = Clamp(obj->vel.x - ACCEL, -MAX_VELOCITY, 0);
    }
    else if (IsKeyDown(KEY_SPACE))
    {
        if (++obj->jmpcnt < MAX_JMP_CNT)
        {
            obj->vel.y = Clamp(obj->vel.y + JMP_ACCL, MAX_JMP_VEL, MAX_GRAVITY);
        }
        else
        {
            obj->vel.y = Clamp(obj->vel.y - JMP_ACCL, MAX_JMP_VEL, MAX_GRAVITY);
        }
    }
    else
    {
        obj->vel.x = obj->vel.x > 0 ? Clamp(obj->vel.x - ACCEL, 0, MAX_VELOCITY) : Clamp(obj->vel.x + ACCEL, -MAX_VELOCITY, 0);
    }
    // obj->vel.y = Clamp(obj->vel.y + ACCEL, obj->ov.ymin, obj->ov.ymax); // gravity always in effect
    obj->position.x = Clamp(obj->position.x + obj->vel.x, 0, 1920);
    obj->position.y = Clamp(obj->position.y + obj->vel.y, 0, 1080);
    if (obj->vel.x < 0)
    {
        obj->reverse = true;
    }
    else if (obj->vel.x > 0)
    {
        obj->reverse = false;
    }
    UpdateSprite((SpriteRect *)obj->curSpriteEl->data);
}

void DrawMapObject(Object *obj)
{
    DrawSprite((SpriteRect *)obj->curSpriteEl->data, obj->position, obj->scale, obj->rotation, obj->reverse);
}

void SetAction(Object *obj, int action)
{
    DListElmt *curSpriteEl = (DListElmt *)dlist_head(obj->sprites);
    SpriteRect *curSprite;
    while (curSpriteEl != NULL)
    {
        if (((SpriteRect *)curSpriteEl->data)->action == action)
        {
            obj->curSpriteEl = curSpriteEl;
            break;
        }
    }
}