#include "mapobject.h"

void DestroyObject(Object *o) {
    DListElmt *sprtEl;
    for (sprtEl = dlist_head(o->sprites); sprtEl != NULL; sprtEl = dlist_next(sprtEl)) {
        list_destroy(((SpriteRect *)sprtEl->data)->colliders);
    }
    dlist_destroy(o->sprites);
    free(o->name);
    free(o);
};

void DestroyCollider(Collider *collider)
{
    free(collider);
}

void UpdateObject(Object *obj)
{
    UpdateSprite(obj->curSpriteEl);
    obj->vel.y = Clamp(obj->vel.y + ACCEL, obj->ov.ymin, obj->ov.ymax); // gravity always in effect
    obj->vel.x = Clamp(obj->vel.x, obj->ov.xmin, obj->ov.xmax);
    obj->position.x += obj->vel.x;
    obj->position.y += obj->vel.y;
    if (obj->vel.x < 0)
    {
        obj->reverse = true;
    }
    else if (obj->vel.x > 0)
    {
        obj->reverse = false;
    }
}

void DrawObject(Object *obj) {
    DrawSprite((SpriteRect *)obj->curSpriteEl->data, obj->position);
}

void SetAction(Object *obj, int action) {
    DListElmt *curSpriteEl = (DListElmt *)dlist_head(obj->sprites);
    SpriteRect *curSprite;
    while (curSpriteEl != NULL) {
        if (((SpriteRect *)curSpriteEl->data)->action == action) {
            obj->curSpriteEl = curSpriteEl;
            break;
        }
    }
}