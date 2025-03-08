// Map.bin is default file
// Order of saved entities:
// 1. Rectangle: world,
// 2. size_t: strlen(objname)
// 3. char *: objname
// 4. int: objtype {Player, Enemy, Item, Projectile, Env}
// 5. Vector2: position
// 6. float: rotation
// 7. float: scale
// 8. int: sprites->size
// 9. foreach sprite
//   a. size_t: strlen(spritename)
//   b. char *: spritename
//   c. size_t: strlen(filename)
//   d. char *: filename
//   e. int: frameCount
//   f. Rectangle: spriteRect
//   g. foreach collider
//     i. int: collider type {RectType, LineType}
//     ii. Rectangle | Line
#include "LoadMap.h"

void LoadMap(DList *objList, Rectangle *world) {
    char *objTypeNames[] = {
        "Player",
        "Enemy",
        "Item",
        "Projectile",
        "Environment",
    };
    char *actTypeNames[] = {
        "NoAction","Attack","Attack2","AttackNoMovement","AttackCombo",
        "AttackComboNoMovement","AttackNoMovement",
        "Crouch","CrouchAttack","CrouchFull","CrouchTransition",
        "CrouchWalk","Dash","Death","DeathNoMovement",
        "Fall","Hit","Idle","Jump","JumpFallInBetween","Roll",
        "Run","Slide","SlideFull","SlideTransitionEnd",
        "SlideTransitionStart","TurnAround","WallClimb",
        "WallClimbNoMovement","WallHang","WallSlide"
    };
    const char *infile = "./Map/Map.bin";
    FILE *in = fopen(infile, "r");
    if (in == NULL) {
        fprintf(stderr, "ERROR: Failed to load file %s\n", infile);
        return;
    }
    int objsSize, oidx, sidx, cidx, sprtsSize, colSize;
    size_t sprtNameLen, shtNameLen, objNameLen;
    
    fread(world, sizeof(Rectangle), 1, in);
    printf("World: {%f, %f, %f, %f}\n", world->x, world->y,
        world->width, world->height);
    fread(&objsSize, sizeof(int), 1, in);
    printf("Object cnt: %d\n", objsSize);
    for (oidx = 0; oidx < objsSize; ++oidx) {
        Object *o1 = (Object *)malloc(sizeof(Object));
        fread(&objNameLen, sizeof(size_t), 1, in);
        o1->name = (char *)malloc(objNameLen + 1);
        o1->name[objNameLen] = '\0';
        fread(o1->name, objNameLen, 1, in);
        fread(&o1->type, sizeof(int), 1, in);
        fread(&o1->position, sizeof(Vector2), 1, in);
        fread(&o1->rotation, sizeof(float), 1, in);
        fread(&o1->scale, sizeof(float), 1, in);
        fread(&sprtsSize, sizeof(int), 1, in);
        o1->alive = true;
        o1->attack = false;
        o1->vel = (Vector2){0.0f, 0.0f};
        o1->ov = (ObjVelLims) {0.0f, 0.0f, world->width, world->height};
        o1->sprites = (DList *)malloc(sizeof(DList));
        dlist_init(o1->sprites, (void *)DestroySprite);
        for (sidx = 0; sidx < sprtsSize; ++sidx) {
            SpriteRect *s1 = (SpriteRect *)malloc(sizeof(SpriteRect));
            fread(&sprtNameLen, sizeof(size_t), 1, in);
            s1->name = (char *)malloc(sprtNameLen + 1);
            fread(s1->name, sprtNameLen, 1, in);
            s1->name[sprtNameLen] = '\0';
            fread(&shtNameLen, sizeof(size_t), 1, in);
            s1->filename = (char *)malloc(shtNameLen + 1);
            fread(s1->filename, shtNameLen, 1, in);
            s1->filename[shtNameLen] = '\0';
            fread(&s1->frameCount, sizeof(float), 1, in);
            fread(&s1->r, sizeof(Rectangle), 1, in);
            fread(&s1->action, sizeof(int), 1, in);
            fread(&colSize, sizeof(int), 1, in);
            s1->colliders = (List *)malloc(sizeof(List));
            list_init(s1->colliders, (void *)DestroyCollider);
            for (cidx = 0; cidx < colSize; ++cidx) {
                Collider *c1 = (Collider *)malloc(sizeof(Collider));
                fread(&c1->type, sizeof(int), 1, in);
                if (c1->type == RectType) {
                    fread(&c1->data.rect, sizeof(Rectangle), 1, in);
                } else if(c1->type == LineType) {
                    fread(&c1->data.line, sizeof(Line), 1, in);
                }
                list_ins_next(s1->colliders, list_tail(s1->colliders), c1);
            }
            dlist_ins_next(o1->sprites, list_tail(o1->sprites), s1);
        }
        o1->curSpriteEl = dlist_head(o1->sprites);
        dlist_ins_next(objList, list_tail(objList), o1);
    }
    // Display the loaded objects
    printf("------------Objects------------\n");
    DListElmt *objEl;
    for (objEl = dlist_head(objList); objEl != NULL; objEl = dlist_next(objEl)) {
        Object *o = (Object *)objEl->data;
        // printf("obj: %s, type: %d, pos: { %f, %f }, rot: %f\n", 
        //     o->name, o->type, o->position.x, o->position.y, o->rotation);
        printf("%s%s, type: %d, pos: { %f, %f }, rot: %f, scl: %f, sprts: %d\n", 
            "obj: ", o->name, o->type, o->position.x, 
            o->position.y, o->rotation, o->scale,
        o->sprites->size);
        DListElmt *sprtEl;
        for (sprtEl = dlist_head(o->sprites); sprtEl != NULL; sprtEl = dlist_next(sprtEl)) {
            SpriteRect *sr1 = dlist_data(sprtEl);
            printf("%*s, %s, r:{%f,%f,%f,%f}, %s\n", 
                (int)(strlen(sr1->name)) + 2, sr1->name, sr1->filename, sr1->r.x,sr1->r.y, sr1->r.width, sr1->r.height, actTypeNames[sr1->action]);
            ListElmt *colEl;
            for (colEl = list_head(sr1->colliders); colEl != NULL; colEl = list_next(colEl)) {
                Collider *c1 = list_data(colEl);
                if (c1->type == RectType) {
                    printf("%*s, {x:%f,y:%f,w:%f,h:%f}\n", (int)(strlen("colRect") + 4), "colRect", c1->data.rect.x, c1->data.rect.y, c1->data.rect.width, c1->data.rect.height);
                } else if (c1->type == LineType) {
                    printf("%*s, {p1:{%f,%f},p2:{%f,%f}}\n", (int)(strlen("colLine") + 4), "colLine", c1->data.line.p1.x, c1->data.line.p1.y, c1->data.line.p2.x, c1->data.line.p2.y);
                }
            }
        }
    }
}