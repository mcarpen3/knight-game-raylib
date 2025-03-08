 #include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../Datastructs/linkedlist.h"
#include "../Datastructs/doublelinkedlist.h"
#include "../Datastructs/set.h"
#include "../util/AutoGetSpriteRect.c"
#include "../Sprite/sprite.h"
#include "../Object/mapobject.h"

#define MAX_INPUT_CHARS 16
#define ARRAY_LEN(x) (sizeof(x) / sizeof((x)[0]))
#define SHEET_PAD 8
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define WORLD_WIDTH (SCREEN_WIDTH) * (4)
#define WORLD_HEIGHT SCREEN_HEIGHT
#define SCALE_FACTOR 0.2f
#define ROT_FACTOR 2.0f
#define MAX_ZOOM 16.0f
#define MIN_ZOOM 0.5f
#define EL_PADDING 6.0f
#define FONTLG 30.0f
#define FONTSM 20.0f
#define CARD_DIM 160.0f

typedef struct SpriteSheet
{
    char *filename;
    char *basename;
    DList *sheetRects;
    Texture t;
    Vector2 offset;
    DListElmt *selectedRect;
} SpriteSheet;

typedef struct Button_
{
    Vector2 position;
    bool clicked;
    bool disabled;
    bool hover;
    const char *text;
    Vector2 padding;
    Rectangle destination;
    float fontSize;
} Button;

void DestroySheet(SpriteSheet *ss);
int SheetMatch(const void *key1, const void *key2) {
    return ((SpriteSheet *)key1)->filename == ((SpriteSheet *)key2)->filename ? 1 : 0;
}
static void DrawLoadedTextures(DList *textures, DListElmt *selected);
static void DrawStoredSheetSpriteRects(SpriteSheet *sheet);
static void DrawStoredObjsList(DList *objectList, DListElmt *selected, char **objtypes);
static void DrawObjectSprites(Object *obj);
static void DrawObjectSpritesMenu(DListElmt *curObjElmt);
static void SelfDestruct(DList *sheetList, List *level, DList *objectList);
static void PlaceObjectInWorld(List *placedObjects, Object *obj);

static void DrawObject(Object *obj);
static void RectInput(Rectangle *rect, Vector2 mouseWorldPos, Rectangle bounds);
static void LineInput(Line *line, Vector2 mouseWorldPos, Rectangle bounds);
static void UpdateMode(int *mode, Rectangle *r);
static bool GetTextInput(char *input, bool *clearBuffer, int *charCount);
static bool GetNumInput(int *input, int min, int max, bool shift);
static void StoreSpriteRect(SpriteSheet *curSheet, Rectangle r, char *name, int frameCount, int action);
static void InitSpriteSheets(const char *folder, DList *sheetList);
static Button InitButton(const char *text, Vector2 position, float fontSize);
static void DrawButton(Button b);
static void UpdateSpriteFrame(SpriteRect *sprite);
static void SaveMap(Rectangle world, List *levelObjList);
static void SelectionCycle(DList *list, DListElmt **current, int direction);


int main(void)
{
    const char *spritesFolder = "/home/matt/Documents/c/raylib/resources";
    DList *sheetList = (DList *)malloc(sizeof(DList));
    Set *activeSheetList = (Set *)malloc(sizeof(Set));
    List *levelObjList = (List *)malloc(sizeof(List));
    DList *objectList = (DList *)malloc(sizeof(DList));
    char *input = (char *)malloc(128);
    char *text = (char *)malloc(MAX_INPUT_CHARS);
    char *info = (char *)malloc(256);
    char *numinput = (char *)malloc(128);
    char *actinput = (char *)malloc(128);
    char *spritePrompt = "SPRITE NAME: ";
    char *framesPrompt = "FRAME COUNT: ";
    char *spriteActPrompt = "SPRITE ACT.: ";
    char *objectPrompt = "OBJ. NAME: ";
    char *typePrompt = "OBJ. TYPE: ";
    char *help[][8] = {
        {
            // mode0 - cut out sprite rects
            "'LEFTCLICK/DRAG'=Cut Out Rect",
            "'R'=Define Sprite Rect",
            "'UP/DOWN'=Cycle Frame Count",
            "'SHIFT+UP/DOWN'=Cycle Actions",
            "'ENTER'=Store Rect.",
            "'PGUP/PGDN'=Change Sheet",
            "'HOME/END'=Select Stored Rect.",
            "'DEL'=Delete Selected Rect.",
        },
        {
            // mode1 - draw colliders on sprite rects
            "'LEFTCLICK/DRAG'=Draw colliders",
            "'L'=Swtich between LINE/RECT",
            "'R'=Store Collider on Sprite",
            "'PGUP/PGDN'=Change Sheet",
            "'HOME/END'=Change Sprite",
        },
        {
            // mode2 - create objectList from 1 or more spriteRects
            "'N'=Create New Object",
            "'R'=Add current sprite rect to the current object",
            "'PGUP/PGDN'=Change Sheet",
            "'HOME/END'=Change Sprite",
            "'LSHIFT+PGUP/PGDN'=Change Object",
            "'UP/DOWN'=Change Object Type",
        },
        {
            // mode3 - place objectList in the map
            "'UP/DOWN'=Scale",
            "'RIGHT/LEFT'=Rotate",
            "'LEFTCLICK'=Place Object",
            "'HOME/END'=Prev/Next Object",
            "'F'=Save map",
        },
    };
    char *commonHelp[] = {
        "'H'=Hide Help",
        "'1-4'=Mode Select"};
    char *modeStrs[] = {
        "RECTS",
        "COLLIDERS",
        "OBJECTS",
        "MAP",
    };
    char *objTypes[] = {
        "Player",
        "Enemy",
        "Item",
        "Projectile",
        "Environment",
    };
    char *actTypes[] = {
        "NoAction","Attack","Attack2","AttackNoMovement","AttackCombo",
        "AttackComboNoMovement","AttackNoMovement",
        "Crouch","CrouchAttack","CrouchFull","CrouchTransition",
        "CrouchWalk","Dash","Death","DeathNoMovement",
        "Fall","Hit","Idle","Jump","JumpFallInBetween","Roll",
        "Run","Slide","SlideFull","SlideTransitionEnd",
        "SlideTransitionStart","TurnAround","WallClimb",
        "WallClimbNoMovement","WallHang","WallSlide"
    };
    float panSpeed = 10.0f;
    int letterCount = 0;
    int number = 1;
    int actNum = 0;
    int mode = 0;
    bool displayHelp = true;
    bool inputMode = false;
    bool lineCollider = false;
    bool clearInput = false;
    bool saving = false;

    dlist_init(sheetList, (void *)DestroySheet);
    set_init(activeSheetList, &SheetMatch, NULL);
    dlist_init(objectList, (void *)DestroyObject);
    list_init(levelObjList, (void *)DestroyObject);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "MAP SPRITE EDITOR!");
    SetTargetFPS(60);
    InitSpriteSheets(spritesFolder, sheetList);

    Vector2 fontvec = MeasureTextEx(GetFontDefault(), "M", FONTLG, 1);
    DListElmt *curSheetElmt = sheetList->head;
    SpriteSheet *curSheet = (SpriteSheet *)curSheetElmt->data;
    DListElmt *curActiveSheetElmt = activeSheetList->head;
    SpriteSheet *curActiveSheet = NULL;
    DListElmt *curObjElmt = objectList->head;
    Camera2D camera = {0};
    Vector2 center = (Vector2){SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
    Rectangle r = (Rectangle){0};
    Rectangle world = (Rectangle){
        .x = 0.0f,
        .y = 0.0f,
        .width = WORLD_WIDTH,
        .height = WORLD_HEIGHT,
    };
    Line l = (Line){0};
    Button autoBtn = InitButton("Create Colliders", (Vector2){
                                                        .x = SCREEN_WIDTH / 2,
                                                        .y = SCREEN_HEIGHT,
                                                    },
                                FONTLG);
    Button rmvColliders = InitButton("Remove Colliders", (Vector2){
                                                             .x = SCREEN_WIDTH / 4,
                                                             .y = SCREEN_HEIGHT,
                                                         },
                                     FONTLG);

    camera.target = (Vector2){center.x, center.y};
    camera.offset = (Vector2){center.x, center.y};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    while (!WindowShouldClose())
    {
        //// UPDATE
        camera.zoom = Clamp(camera.zoom + (float)GetMouseWheelMove() * 0.2f, MIN_ZOOM, MAX_ZOOM);

        // Get the world point that is under the mouse
        Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

        // Set the offset to where the mous}e is
        camera.offset = GetMousePosition();

        // Set the camera target to match, so that the camera maps the world space point
        // under the cursor to the screen space point under the cursor at any zoom
        camera.target = mouseWorldPos;

        if (inputMode)
        {
            if (GetTextInput(text, &clearInput, &letterCount) == true)
            {
                inputMode = false;
                if (mode == 0)
                {
                    StoreSpriteRect(curSheet, r, text, number, actNum);
                    set_insert(activeSheetList, curSheet);
                    curActiveSheetElmt = dlist_tail(activeSheetList);
                }
                if (mode == 2)
                {
                    Object *newObj = (Object *)malloc(sizeof(Object));
                    newObj->sprites = (DList *)malloc(sizeof(DList));
                    dlist_init(newObj->sprites, NULL);
                    newObj->position = (Vector2){0};
                    newObj->rotation = 0.0f;
                    newObj->scale = 1.0f;
                    newObj->name = strdup(text);
                    newObj->curSpriteEl = NULL;
                    newObj->type = number;
                    dlist_ins_next(objectList, dlist_tail(objectList), (void *)newObj);
                    curObjElmt = dlist_tail(objectList);
                }
                memset(text, 0, MAX_INPUT_CHARS);
                letterCount = 0;
            }
            GetNumInput(&number, mode == 0 ? 1 : 0, mode == 0 ? 60 : ARRAY_LEN(objTypes) - 1, false);
            sprintf(input, "%s%s", mode == 0 ? spritePrompt : objectPrompt, text);
            if (mode == 0)
            {
                GetNumInput(&actNum, 0, ARRAY_LEN(actTypes) - 1, true);
                sprintf(numinput, "%s%d", framesPrompt, number);
                sprintf(actinput, "%s%s", spriteActPrompt, actTypes[actNum]);
            }
            if (mode == 2)
            {
                sprintf(numinput, "%s%s", typePrompt, objTypes[number]);
            }
        }
        else
        {
            UpdateMode(&mode, &r);
            if (IsKeyDown(KEY_A))
            {
                camera.offset.x += panSpeed;
            }
            if (IsKeyDown(KEY_D))
            {
                camera.offset.x -= panSpeed;
            }
            if (IsKeyDown(KEY_W))
            {
                camera.offset.y += panSpeed;
            }
            if (IsKeyDown(KEY_S))
            {
                camera.offset.y -= panSpeed;
            }
            if (IsKeyPressed(KEY_H))
            {
                displayHelp = !displayHelp;
            }
            if (IsKeyPressed(KEY_PAGE_UP) || IsKeyPressed(KEY_PAGE_DOWN)) {
                int key = GetKeyPressed();
                if (mode == 0) {
                    SelectionCycle(sheetList, &curSheetElmt, key);
                } else if (mode == 2 && IsKeyDown(KEY_LEFT_SHIFT)) {
                    SelectionCycle(objectList, &curObjElmt, key);
                } else if (mode != 3) {
                    SelectionCycle(activeSheetList, &curActiveSheetElmt, key);
                } else if (curObjElmt) {
                    SelectionCycle(objectList, &curObjElmt, key);
                }
                r.width = 0.0f;
                r.height = 0.0f;
                r.x = 0.0f;
                r.y = 0.0f;
            }
            
            if (IsKeyPressed(KEY_HOME) || IsKeyPressed(KEY_END))
            {
                int key = GetKeyPressed();
                if (mode == 0) {
                    SelectionCycle(curSheet->sheetRects, &curSheet->selectedRect, key);
                } else if (mode != 3) {
                    SelectionCycle(curActiveSheet->sheetRects, &curActiveSheet->selectedRect, key);
                } else if (curObjElmt) {
                    Object *tmpobj = curObjElmt->data;
                    SelectionCycle(tmpobj->sprites, &tmpobj->curSpriteEl, key);
                }
            }

            if (mode == 0)
            {
                // sprite slice mode
                curSheet = (SpriteSheet *)curSheetElmt->data;
                sprintf(info, "MODE: %s, File: \"%s\", Zoom: [%.2f]", modeStrs[mode], curSheet->basename, camera.zoom);
                RectInput(&r, mouseWorldPos,
                          (Rectangle){
                              curSheet->offset.x,
                              curSheet->offset.y,
                              curSheet->t.width,
                              curSheet->t.height,
                          });
                if (IsKeyPressed(KEY_R))
                {
                    number = 1;
                    actNum = 0;
                    inputMode = true;
                    clearInput = true;
                }
                if (IsKeyPressed(KEY_DELETE))
                {
                    SpriteRect *tmpElmtData = (SpriteRect *)malloc(sizeof(SpriteRect));
                    tmpElmtData->name = (char *)malloc(64);
                    if ((dlist_remove(curSheet->sheetRects, curSheet->selectedRect, (void *)&tmpElmtData)) != -1)
                    {
                        curSheet->selectedRect = curSheet->sheetRects->head;
                        printf("deleted %s\n", tmpElmtData->name);
                        free(tmpElmtData->name);
                        free(tmpElmtData);
                    }
                }
            }
            if (mode == 1)
            {
                if (activeSheetList->size > 0)
                {
                    // sprite colliders mode
                    curActiveSheet = (SpriteSheet *)curActiveSheetElmt->data;
                    sprintf(info, "MODE: %s, File: \"%s\", Zoom: [%.2f], CldrMode: %s",
                            modeStrs[mode],
                            curActiveSheet->basename,
                            camera.zoom,
                            lineCollider ? "LINE" : "RECT");
                    autoBtn.disabled = false;
                    SpriteRect *curRect = (SpriteRect *)curActiveSheet->selectedRect->data;
                    Rectangle curRectBounds = (Rectangle){
                        .x = SCREEN_WIDTH / 2 - curRect->r.width / 2,
                        .y = SCREEN_HEIGHT / 2 - curRect->r.height / 2,
                        .width = curRect->r.width,
                        .height = curRect->r.height,
                    };
                    if (lineCollider == false)
                    {
                        RectInput(&r, mouseWorldPos, curRectBounds);
                    }
                    else
                    {
                        LineInput(&l, mouseWorldPos, curRectBounds);
                    }
                    if (curRect->colliders->size > 0)
                    {
                        rmvColliders.disabled = false;
                    }
                    if (IsKeyPressed(KEY_R))
                    {
                        printf("INFO: Saving collider..\n");
                        SpriteRect *sptr = (SpriteRect *)curActiveSheet->selectedRect->data;
                        Collider *tmpCldr = (Collider *)malloc(sizeof(Collider));
                        Vector2 spriteOffset = (Vector2){
                            .x = SCREEN_WIDTH / 2 - sptr->r.width / 2,
                            .y = SCREEN_HEIGHT / 2 - sptr->r.height / 2,
                        };
                        if (lineCollider == false)
                        {
                            tmpCldr->type = RectType;
                            tmpCldr->data.rect = (Rectangle){
                                .x = r.x - spriteOffset.x,
                                .y = r.y - spriteOffset.y,
                                .width = r.width,
                                .height = r.height,
                            };
                            list_ins_next(sptr->colliders, list_tail(sptr->colliders), tmpCldr);
                        }
                        else
                        {
                            tmpCldr->type = LineType;
                            tmpCldr->data.line.p1 = (Vector2){.x = l.p1.x, .y = l.p1.y};
                            tmpCldr->data.line.p2 = (Vector2){.x = l.p2.x, .y = l.p2.y};
                            list_ins_next(sptr->colliders, list_tail(sptr->colliders), tmpCldr);
                        }
                    }
                    if (CheckCollisionPointRec(GetMousePosition(), autoBtn.destination))
                    {
                        autoBtn.hover = true;
                        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && autoBtn.disabled == false)
                        {
                            SpriteRect *sptr = (SpriteRect *)curActiveSheet->selectedRect->data;
                            list_init(sptr->colliders, (void *)DestroyCollider);
                            GetSpriteBounds(LoadImageFromTexture(sptr->texture), sptr->colliders);
                            printf("INFO: auto bounds found %d rects\n", sptr->colliders->size);
                        }
                    }
                    else
                    {
                        autoBtn.hover = false;
                    }
                    if (CheckCollisionPointRec(GetMousePosition(), rmvColliders.destination))
                    {
                        rmvColliders.hover = true;
                        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && rmvColliders.disabled == false)
                        {
                            SpriteRect *sptr = (SpriteRect *)curActiveSheet->selectedRect->data;
                            printf("INFO: Removing %d\n", sptr->colliders->size);
                            list_destroy(sptr->colliders);
                            list_init(sptr->colliders, (void *)DestroyCollider);
                        }
                    }
                    else
                    {
                        rmvColliders.hover = false;
                    }
                }
                if (IsKeyPressed(KEY_L))
                {
                    lineCollider = !lineCollider;
                }
            }
            if (mode == 2)
            {
                if (activeSheetList->size > 0) {
                    // Object making mode
                    curActiveSheet = (SpriteSheet *)curActiveSheetElmt->data;
                    sprintf(info, "MODE: %s, File: \"%s\", Zoom: [%.2f]", modeStrs[mode], curActiveSheet->basename, camera.zoom);
                    if (IsKeyPressed(KEY_N))
                    {
                        inputMode = true;
                        clearInput = true;
                        number = 0;
                    }
                    if (IsKeyPressed(KEY_R))
                    {
                        // add the current spriteRect to the object
                        Object *curObj = (Object *)curObjElmt->data;
                        SpriteRect *curRect = (SpriteRect *)curActiveSheet->selectedRect->data;
                        dlist_ins_next(curObj->sprites, dlist_tail(curObj->sprites), curRect);
                        curObj->curSpriteEl = dlist_tail(curObj->sprites);
                    }
                }
            }
            if (mode == 3)
            {
                // object place mode
                sprintf(info, "MODE: %s, ObjName: \"%s\", Zoom: [%.2f], Scale: [%.2f]", modeStrs[mode],
                        curObjElmt == NULL ? "No objects!" : ((Object *)curObjElmt->data)->name,
                        camera.zoom, ((Object *)curObjElmt->data)->scale);
                if (curObjElmt != NULL)
                {
                    Object *curObj = (Object *)curObjElmt->data;
                    DListElmt *placedObject = dlist_head(objectList);
                    while (placedObject != NULL)
                    {
                        Object *tmpobj = (Object *)placedObject->data;
                        if (tmpobj->curSpriteEl != NULL)
                        {
                            SpriteRect *tmprect = (SpriteRect *)tmpobj->curSpriteEl->data;
                            UpdateSpriteFrame(tmprect);
                        }
                        placedObject = placedObject->next;
                    }
                    if (curObj->sprites->size > 0)
                    {
                        SpriteRect *tmprect = (SpriteRect *)curObj->curSpriteEl->data;
                        curObj->position.x = Clamp(mouseWorldPos.x,
                                                   tmprect->r.width * curObj->scale / tmprect->frameCount / 2,
                                                   WORLD_WIDTH - tmprect->r.width * curObj->scale / tmprect->frameCount / 2);
                        curObj->position.y = Clamp(mouseWorldPos.y,
                                                   tmprect->r.height * curObj->scale / 2,
                                                   WORLD_HEIGHT - tmprect->r.height * curObj->scale / 2);
                        if (IsKeyDown(KEY_UP))
                        {
                            curObj->scale = Clamp(curObj->scale + SCALE_FACTOR, 1.0f, 10.0f);
                        }
                        if (IsKeyDown(KEY_DOWN))
                        {
                            curObj->scale = Clamp(curObj->scale - SCALE_FACTOR, 1.0f, 10.0f);
                        }
                        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                        {
                            PlaceObjectInWorld(levelObjList, curObj);
                        }
                        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
                        {
                            // drag the mouse to place more sprites adjacent to the currently placed one
                            ListElmt *last = list_tail(levelObjList);
                            Object *placedObj = (Object *)last->data;
                            SpriteRect *placed = (SpriteRect *)placedObj->curSpriteEl->data;
                            SpriteRect *rect = (SpriteRect *)((Object *)curObjElmt->data)->curSpriteEl->data;
                            Rectangle cr = (Rectangle){
                                .width = rect->r.width * curObj->scale,
                                .height = rect->r.height * curObj->scale,
                                .x = curObj->position.x - (rect->r.width * curObj->scale / 2),
                                .y = curObj->position.y - (rect->r.height * curObj->scale / 2),
                            };
                            Rectangle cp = (Rectangle){
                                .width = placed->r.width * placedObj->scale,
                                .height = placed->r.height * placedObj->scale,
                                .x = placedObj->position.x - (placed->r.width * placedObj->scale / 2),
                                .y = placedObj->position.y - (placed->r.height * placedObj->scale / 2)};
                            if (!CheckCollisionRecs(cr, cp))
                            {
                                PlaceObjectInWorld(levelObjList, curObj);
                            }
                            if (IsKeyPressed(KEY_HOME))
                            {
                                if (dlist_is_head(curObjElmt))
                                {
                                    curObjElmt = dlist_tail(objectList);
                                }
                                else
                                {
                                    curObjElmt = curObjElmt->prev;
                                }
                            }
                            if (IsKeyPressed(KEY_END))
                            {
                                if (dlist_is_tail(curObjElmt))
                                {
                                    curObjElmt = dlist_tail(objectList);
                                }
                                else
                                {
                                    curObjElmt = curObjElmt->next;
                                }
                            }
                        }
                        if (IsKeyPressed(KEY_F) && levelObjList->size > 0)
                        {
                            SaveMap(world, levelObjList);
                        }
                        if (IsKeyPressed(KEY_H))
                        {
                            curObj->rotation = 0.0f;
                        }
                        if (IsKeyDown(KEY_RIGHT))
                        {
                            curObj->rotation = curObj->rotation + ROT_FACTOR > 359.0 ? 0.0f : curObj->rotation + ROT_FACTOR;
                        }
                    }
                }
            }
        }

        //// DRAW
        BeginDrawing();
        ClearBackground(DARKGRAY);
        BeginMode2D(camera);
        if (mode == 0)
        {
            // Sprite slice mode
            // draw a grid
            for (int i = curSheet->offset.x; i <= curSheet->offset.x + curSheet->t.width; i += 10)
            {
                DrawLine(i, curSheet->offset.y, i, curSheet->offset.y + curSheet->t.height, BLACK);
            }
            for (int i = curSheet->offset.y; i <= curSheet->offset.y + curSheet->t.height; i += 10)
            {
                DrawLine(curSheet->offset.x, i, curSheet->offset.x + curSheet->t.width, i, BLACK);
            }
            DrawRectangleLines(curSheet->offset.x, curSheet->offset.y,
                               curSheet->t.width, curSheet->t.height, BLACK);
            // draw the current texture to the middle of the screen
            DrawTexture(curSheet->t, curSheet->offset.x, curSheet->offset.y, WHITE);
            DrawRectangleLinesEx(r, 1.0f, GREEN);
        }
        if (mode == 1)
        {
            // Collider mode
            // Draw the currently selected sprite in the middle of the screen
            if (activeSheetList->size > 0) {
                SpriteRect *tmprect = (SpriteRect *)curActiveSheet->selectedRect->data;
                Rectangle destination = (Rectangle){
                    .x = (SCREEN_WIDTH / 2) - (tmprect->r.width / 2),
                    .y = (SCREEN_HEIGHT / 2) - (tmprect->r.height / 2),
                    .width = tmprect->r.width,
                    .height = tmprect->r.height};
                DrawTexturePro(tmprect->texture, tmprect->r, destination, (Vector2){0}, 0.0f, WHITE);
                DrawRectangleLinesEx(destination, 1.0f, BLACK);
                ListElmt *i = (ListElmt *)tmprect->colliders->head;
                while (i != NULL)
                {
                    Collider *tmp = (Collider *)i->data;
                    if (tmp->type == RectType)
                    {
                        Rectangle out = (Rectangle){
                            .x = destination.x + tmp->data.rect.x,
                            .y = destination.y + tmp->data.rect.y,
                            .width = tmp->data.rect.width,
                            .height = tmp->data.rect.height,
                        };
                        DrawRectangleLinesEx(out, 1.0f, GREEN);
                    }
                    if (tmp->type == LineType)
                    {
                        DrawLineBezier(tmp->data.line.p1, tmp->data.line.p2, 1.0f, GREEN);
                    }
                    i = i->next;
                }
            }
            if (lineCollider) {
                DrawLineBezier(l.p1, l.p2, 1.0, GREEN);
            } else {
                DrawRectangleLinesEx(r, 1.0f, GREEN);
            }
        }
        if (mode == 2)
        {
            // Object collection mode
        }
        if (mode == 3)
        {
            // draw the world rectangle
            DrawRectangleRec(world, BROWN);
            // draw the saved objectList
            ListElmt *levelEl = levelObjList->head;
            while (levelEl != NULL)
            {
                Object *curObj = (Object *)levelEl->data;
                DrawObject(curObj);
                levelEl = levelEl->next;
            }
            if (curObjElmt != NULL)
            {
                Object *curObj = (Object *)curObjElmt->data;
                DrawObject(curObj);
            }
        }
        EndMode2D();
        // Draw Crosshairs
        DrawLine(SCREEN_WIDTH / 2, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT, BLACK);
        DrawLine(0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2, BLACK);

        DrawText(info, 4, 0, FONTLG, GOLD);

        if (saving)
        {
            DrawText("Saving...", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, FONTLG, BLUE);
        }

        if (displayHelp)
        {
            float curse = 0;
            for (int i = 0; i < (int)ARRAY_LEN(help[mode]); ++i)
            {
                curse = 40 + (i * (FONTLG + 10));
                if (help[mode][i] != NULL)
                    DrawText(help[mode][i], 4, curse, FONTLG, RAYWHITE);
            }
            curse += FONTLG + 10;
            for (int i = 0; i < (int)ARRAY_LEN(commonHelp); ++i)
            {
                DrawText(commonHelp[i], 4, curse + (i * (FONTLG + 10)), FONTLG, RAYWHITE);
            }
        }
        if (mode == 0)
        {
            DrawLoadedTextures(sheetList, curSheetElmt);
            DrawStoredSheetSpriteRects(curSheet);
        }
        else if (mode != 3)
        {
            DrawLoadedTextures(activeSheetList, curActiveSheetElmt);
            DrawStoredSheetSpriteRects(curActiveSheet);
        }
        if (mode == 1)
        {
            if (!autoBtn.disabled)
                DrawButton(autoBtn);
            if (!rmvColliders.disabled)
                DrawButton(rmvColliders);
        }
        if (mode == 2)
        {
            if (curObjElmt != NULL)
            {
                Object *curObj = (Object *)curObjElmt->data;
                DrawObjectSprites(curObj);
            }
        }
        if (mode == 2 || mode == 3)
        {
            DrawStoredObjsList(objectList, curObjElmt, objTypes);
        }
        if (mode == 3)
        {
            DrawObjectSpritesMenu(curObjElmt);
        }
        if (inputMode)
        {
            Rectangle input1 = (Rectangle){
                .x = 4,
                .y = SCREEN_HEIGHT - FONTLG,
                .width = (strlen(spritePrompt) + MAX_INPUT_CHARS) * fontvec.x,
                .height = FONTLG + 2,
            };
            DrawText(input, input1.x + 2, input1.y + 2, FONTLG, WHITE);
            DrawRectangleLines(input1.x, input1.y, input1.width, input1.height, GREEN);
            Rectangle input2 = (Rectangle){
                .x = input1.width + 2,
                .y = input1.y,
                .width = fontvec.x * strlen(numinput),
                .height = input1.height,
            };
            DrawText(numinput, input2.x + 2, input2.y, FONTLG, WHITE);
            DrawRectangleLines(input2.x, input2.y, input2.width, input2.height, GREEN);
            if (mode == 0) {
                Rectangle input3 = (Rectangle) {
                    .x = input1.width + input2.width + 4,
                    .y = input1.y,
                    .width = fontvec.x *strlen(actinput),
                    .height = input1.height,
                };
                DrawText(actinput, input3.x + 2, input3.y, FONTLG, WHITE);
                DrawRectangleLines(input3.x, input3.y, input3.width, input3.height, GREEN);
            }
        }
        EndDrawing();
    }
    SelfDestruct(sheetList, levelObjList, objectList);
    free(info);
    free(input);
    free(numinput);
}

void DestroySheet(SpriteSheet *ss)
{
    printf("INFO: SpriteSheet %s destroying\n", ss->filename);
    UnloadTexture(ss->t);
    free(ss->filename);
    free(ss->basename);
    free(ss);
}

static void DrawStoredObjsList(DList *objectList, DListElmt *selected, char **objtypes)
{
    float yidx = 0.0f;
    float objFontSz = 20.0f;
    float rmargin = CARD_DIM + EL_PADDING;
    int curIdx = 0;
    Vector2 objFontPad = {.x = EL_PADDING, .y = EL_PADDING};
    const char *nomsg = "No objectList!";
    Vector2 fontvec = MeasureTextEx(GetFontDefault(), nomsg, 20.0f, 2.0f);
    if (objectList->size == 0)
    {
        DrawText(nomsg, GetScreenWidth() - rmargin - fontvec.x - objFontPad.x, yidx, 20, WHITE);
    }
    DListElmt *cursor = dlist_head(objectList);
    while (cursor != selected)
    {
        curIdx++;
        cursor = cursor->next;
    }
    if (curIdx * fontvec.y > SCREEN_HEIGHT)
    {
        yidx = -(curIdx * fontvec.y - SCREEN_HEIGHT);
    }
    cursor = dlist_head(objectList);
    while (cursor != NULL)
    {
        bool sel = selected == cursor ? true : false;
        Object *obj = (Object *)cursor->data;
        char *output = (char *)malloc(64);
        sprintf(output, "%s SPRITES: %d, TYPE: %s", obj->name, obj->sprites->size, objtypes[obj->type]);
        Vector2 fontvec = MeasureTextEx(GetFontDefault(), output, objFontSz, 2.0f);
        Rectangle box = (Rectangle){
            .x = SCREEN_WIDTH - rmargin - fontvec.x - objFontPad.x * 2,
            .y = yidx,
            .width = fontvec.x + objFontPad.x * 2,
            .height = fontvec.y + objFontPad.y,
        };
        DrawRectangleLinesEx(box, sel ? 2.0f : 1.0f, sel ? GREEN : BLACK);
        DrawText(output, box.x + objFontPad.x, box.y + objFontPad.y, objFontSz, WHITE);
        cursor = cursor->next;
        yidx += box.height;
        free(output);
    }
}

static void DrawObjectSprites(Object *obj)
{
    float yidx2 = 30.0f;
    DListElmt *el = dlist_head(obj->sprites);
    while (el != NULL)
    {
        SpriteRect *sprite = (SpriteRect *)el->data;
        Rectangle dest = (Rectangle){
            .x = SCREEN_WIDTH / 2 - sprite->r.width / 2,
            .y = yidx2,
            .width = sprite->r.width,
            .height = sprite->r.height,
        };
        DrawTexturePro(sprite->texture, sprite->r, dest, (Vector2){0}, 0.0f, WHITE);
        DrawRectangleLinesEx(dest, 1.0f, BLACK);
        DrawText(sprite->name, dest.x + EL_PADDING, dest.y + EL_PADDING, FONTSM, WHITE);
        yidx2 += sprite->r.height + EL_PADDING;
        el = el->next;
    }
}

static void DrawObjectSpritesMenu(DListElmt *curObjElmt)
{
    float outdim = CARD_DIM;
    int curIdx = 1;
    float yidx = 0.0f;
    float xrule = SCREEN_WIDTH - outdim;
    if (curObjElmt == NULL || ((Object *)curObjElmt->data)->sprites->size == 0)
    {
        DrawText("No sprites!", xrule, yidx, 20, WHITE);
        return;
    }
    Object *tmpobj = (Object *)curObjElmt->data;
    DListElmt *el = tmpobj->sprites->head;
    while (el != tmpobj->curSpriteEl)
    {
        curIdx++;
        el = el->next;
    }
    if (curIdx * outdim > SCREEN_HEIGHT)
    {
        yidx = -(curIdx * outdim - SCREEN_HEIGHT);
    }
    el = tmpobj->sprites->head;
    while (el != NULL)
    {
        bool sel = el == tmpobj->curSpriteEl ? true : false;
        SpriteRect *sr = (SpriteRect *)el->data;

        Rectangle src = (Rectangle){
            .x = sr->r.x,
            .y = sr->r.y,
            .width = sr->r.width,
            .height = sr->r.height,
        };
        float scaled = outdim / fmaxf(src.width, src.height);
        Rectangle dest = (Rectangle){
            .x = xrule + outdim / 2 - (src.width * scaled / 2),
            .y = yidx + outdim / 2 - (src.height * scaled / 2),
            .width = src.width * scaled,
            .height = src.height * scaled,
        };
        DrawTexturePro(sr->texture, src, dest, (Vector2){0}, 0.0f, WHITE);
        DrawRectangleLinesEx((Rectangle){
                                 .x = SCREEN_WIDTH - outdim,
                                 .y = yidx,
                                 .width = outdim,
                                 .height = outdim},
                             sel ? 2.0f : 1.0f, sel ? GREEN : BLACK);
        DrawText(sr->name, SCREEN_WIDTH - outdim + EL_PADDING, yidx + EL_PADDING, 20, WHITE);
        yidx += outdim;
        el = el->next;
    }
}
static void DrawStoredSheetSpriteRects(SpriteSheet *sheet)
{
    float outdim = CARD_DIM;
    int curIdx = 1;
    float yidx = 0.0f;
    float xrule = SCREEN_WIDTH - outdim;
    if (sheet->sheetRects->size == 0)
    {
        DrawText("No rects!", xrule, yidx, 20, WHITE);
        return;
    }
    DListElmt *el = sheet->sheetRects->head;
    while (el != sheet->selectedRect)
    {
        curIdx++;
        el = el->next;
    }
    if (curIdx * outdim > SCREEN_HEIGHT)
    {
        yidx = -(curIdx * outdim - SCREEN_HEIGHT);
    }
    el = sheet->sheetRects->head;
    while (el != NULL)
    {
        bool sel = el == sheet->selectedRect ? true : false;
        SpriteRect *sr = (SpriteRect *)el->data;

        Rectangle src = (Rectangle){
            .x = sr->r.x,
            .y = sr->r.y,
            .width = sr->r.width,
            .height = sr->r.height,
        };
        float scaled = outdim / fmaxf(src.width, src.height);
        Rectangle dest = (Rectangle){
            .x = xrule + outdim / 2 - (src.width * scaled / 2),
            .y = yidx + outdim / 2 - (src.height * scaled / 2),
            .width = src.width * scaled,
            .height = src.height * scaled,
        };
        DrawTexturePro(sheet->t, src, dest, (Vector2){0}, 0.0f, WHITE);
        DrawRectangleLinesEx((Rectangle){
                                 .x = SCREEN_WIDTH - outdim,
                                 .y = yidx,
                                 .width = outdim,
                                 .height = outdim},
                             sel ? 2.0f : 1.0f, sel ? GREEN : BLACK);
        DrawText(sr->name, SCREEN_WIDTH - outdim + EL_PADDING, yidx + EL_PADDING, 20, WHITE);
        char *cldrtxt = (char*)malloc(16);
        Vector2 fontvec = MeasureTextEx(GetFontDefault(), sr->name, FONTSM, 2.0f);
        sprintf(cldrtxt, "Colliders: %d", sr->colliders->size);
        DrawText(cldrtxt, SCREEN_WIDTH - outdim + EL_PADDING, yidx + fontvec.y + EL_PADDING, FONTSM, WHITE);
        yidx += outdim;
        el = el->next;
        free(cldrtxt);
    }
}

static void SelfDestruct(DList *sheetList, List *level, DList *objectList)
{
    DListElmt *cursor = sheetList->head;
    while (cursor != NULL)
    {
        SpriteSheet *curSheet = (SpriteSheet *)cursor->data;
        dlist_destroy(curSheet->sheetRects);
        cursor = cursor->next;
    }
    dlist_destroy(sheetList);
    list_destroy(level);
    dlist_destroy(objectList);
}

static void PlaceObjectInWorld(List *placedObjects, Object *obj)
{
    Object *objcpy = (Object *)malloc(sizeof(Object));
    objcpy->position = obj->position;
    objcpy->scale = obj->scale;
    objcpy->name = obj->name;
    objcpy->type = obj->type;
    objcpy->rotation = obj->rotation;
    objcpy->sprites = obj->sprites;
    objcpy->curSpriteEl = obj->curSpriteEl;
    list_ins_next(placedObjects, list_tail(placedObjects), objcpy);
}

static void UpdateSpriteFrame(SpriteRect *sprite)
{
    if (sprite->frameCount > 1)
    {
        sprite->frameCounter++;
        if (sprite->frameCounter > (60 / sprite->frameCount))
        {
            sprite->frameCounter = 0;
            sprite->frameIdx++;
            if (sprite->frameIdx + 1 > sprite->frameCount)
            {
                sprite->frameIdx = 0;
            }
        }
    }
}

static void DrawObject(Object *obj)
{
    if (obj->curSpriteEl == NULL)
    {
        return;
    }
    SpriteRect *sprite = (SpriteRect *)obj->curSpriteEl->data;

    Rectangle destination = (Rectangle){
        .width = sprite->r.width * obj->scale / sprite->frameCount,
        .height = sprite->r.height * obj->scale,
        .x = obj->position.x - sprite->r.width * obj->scale / sprite->frameCount / 2,
        .y = obj->position.y - sprite->r.height * obj->scale / 2,
    };

    Vector2 origin = (Vector2){0};

    Rectangle dr = (Rectangle){
        .x = sprite->r.x + (sprite->r.width / sprite->frameCount * sprite->frameIdx),
        .y = sprite->r.y,
        .width = sprite->r.width / sprite->frameCount,
        .height = sprite->r.height,
    };
    DrawRectangleLinesEx(destination, 1.0f, GREEN);
    DrawTexturePro(sprite->texture, dr, destination, origin, obj->rotation, WHITE);
}

static void RectInput(Rectangle *r, Vector2 mouseWorldPos, Rectangle bounds)
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        r->x = Clamp(mouseWorldPos.x, bounds.x, bounds.x + bounds.width);
        r->y = Clamp(mouseWorldPos.y, bounds.y, bounds.y + bounds.height);
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        r->width = Clamp(mouseWorldPos.x - r->x, 0.0f, bounds.x + bounds.width - r->x);
        r->height = Clamp(mouseWorldPos.y - r->y, 0.0f, bounds.y + bounds.height - r->y);
    }
}

static void LineInput(Line *line, Vector2 mouseWorldPos, Rectangle bounds)
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        line->p1.x = Clamp(mouseWorldPos.x, bounds.x, bounds.x + bounds.width);
        line->p1.y = Clamp(mouseWorldPos.y, bounds.y, bounds.y + bounds.height);
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        line->p2.x = Clamp(mouseWorldPos.x, bounds.x, bounds.x + bounds.width);
        line->p2.y = Clamp(mouseWorldPos.y, bounds.y, bounds.y + bounds.height);
    }
}

static void UpdateMode(int *mode, Rectangle *r)
{
    if (IsKeyPressed(KEY_ONE))
    {
        *mode = 0;
    }
    if (IsKeyPressed(KEY_TWO))
    {
        r->width = 0;
        r->height = 0;
        *mode = 1;
    }
    if (IsKeyPressed(KEY_THREE))
    {
        *mode = 2;
    }
    if (IsKeyPressed(KEY_FOUR))
    {
        *mode = 3;
    }
}

static bool GetTextInput(char *input, bool *clearBuffer, int *charCount)
{
    char key;
    if (*clearBuffer)
    {
        key = GetCharPressed();
        while (key > 0)
        {
            key = GetCharPressed();
        }
        *clearBuffer = false;
    }
    if (IsKeyPressed(KEY_ENTER) && *charCount > 0)
    {
        *charCount = 0;
        *clearBuffer = false;
        return true;
    }
    // Get char pressed (unicode character) on the queue
    key = GetCharPressed();
    // Check if more characters have been pressed on the same frame
    while (key > 0)
    {
        // NOTE: Only allow keys in range [32..125]
        if ((key >= 32) && (key <= 125) && (*charCount < MAX_INPUT_CHARS))
        {
            input[*charCount] = (char)key;
            input[*charCount + 1] = '\0'; // Add null terminator at the end of the string.
            (*charCount)++;
        }
        key = GetCharPressed(); // Check next character in the queue
    }

    if (IsKeyPressed(KEY_BACKSPACE))
    {
        (*charCount)--;
        if (*charCount < 0)
            *charCount = 0;
        input[*charCount] = '\0';
    }
    return false;
}

static bool GetNumInput(int *input, int min, int max, bool shift)
{
    if ((shift && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_UP)) || 
        (!shift && !IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_UP)))
    {
        *input = Clamp(*input + 1, min, max);
    }
    if ((shift && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_DOWN)) || 
        (!shift && !IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_DOWN)))
    {
        *input = Clamp(*input - 1, min, max);
    }
    if (IsKeyPressed(KEY_ENTER)) {
        return true;
    }
    return false;
}
static void StoreSpriteRect(SpriteSheet *curSheet, Rectangle r, char *name, int frameCount, int action)
{

    SpriteRect *newRect = (SpriteRect *)malloc(sizeof(SpriteRect));
    newRect->colliders = (List *)malloc(sizeof(List));
    list_init(newRect->colliders, (void *)DestroyCollider);
    newRect->r = (Rectangle){
        .x = r.x - curSheet->offset.x,
        .y = r.y - curSheet->offset.y,
        .width = r.width,
        .height = r.height,
    };
    newRect->name = strdup(name);
    newRect->filename = strdup(curSheet->filename);
    newRect->frameCount = frameCount;
    newRect->frameCounter = 0;
    newRect->frameIdx = 0;
    newRect->texture = curSheet->t;
    newRect->action = action;
    dlist_ins_next(curSheet->sheetRects, dlist_tail(curSheet->sheetRects), newRect);
    curSheet->selectedRect = dlist_tail(curSheet->sheetRects);
}

static void InitSpriteSheets(const char *folder, DList *sheetList)
{
    FilePathList spritesheets = LoadDirectoryFilesEx(folder, ".png", true);
    // initialize all the spritesheets
    for (int i = 0; i < (int)spritesheets.count; ++i)
    {
        if (strcmp(GetFileExtension(spritesheets.paths[i]), ".png"))
            continue;
        SpriteSheet *sheet = (SpriteSheet *)malloc(sizeof(SpriteSheet));
        sheet->filename = strdup(spritesheets.paths[i]);
        sheet->basename = strdup(GetFileName(spritesheets.paths[i]));
        sheet->t = LoadTexture(spritesheets.paths[i]);
        sheet->selectedRect = NULL;
        DList *rectlist = (DList *)malloc(sizeof(DList));
        dlist_init(rectlist, (void *)DestroySprite);
        sheet->sheetRects = rectlist;
        sheet->offset = (Vector2){
            SCREEN_WIDTH / 2 - sheet->t.width / 2,
            SCREEN_HEIGHT / 2 - sheet->t.height / 2};
        dlist_ins_next(sheetList, dlist_tail(sheetList), sheet);
    }
}

static Button InitButton(const char *text, Vector2 position, float fontSize)
{
    Vector2 fontvec = MeasureTextEx(GetFontDefault(), text, fontSize, 1);
    Button b = (Button){
        .text = text,
        .clicked = false,
        .hover = false,
        .disabled = false,
        .fontSize = fontSize,
        .padding = (Vector2){.x = EL_PADDING, .y = EL_PADDING},
        .position = position,
    };
    b.destination = (Rectangle){
        .x = position.x - (fontvec.x / 2 - b.padding.x),
        .y = position.y - (fontvec.y + b.padding.y * 2),
        .width = fontvec.x + (b.padding.x * 2),
        .height = fontvec.y + (b.padding.y * 2)};
    return b;
}

static void DrawButton(Button b)
{
    DrawRectangleLinesEx(b.destination, 2.0f, WHITE);
    Vector2 shift = (Vector2){
        .x = b.destination.x + b.padding.x,
        .y = b.destination.y + b.padding.y,
    };
    if (b.hover)
    {
        DrawRectangleRec(b.destination, WHITE);
        DrawTextPro(GetFontDefault(), b.text, shift, (Vector2){0, 0}, 0.0f, b.fontSize, 1.0f, BLACK);
    }
    else
    {
        DrawTextPro(GetFontDefault(), b.text, shift, (Vector2){0, 0}, 0.0f, b.fontSize, 1.0f, WHITE);
    }
}

static void DrawLoadedTextures(DList *textures, DListElmt *selected)
{
    DListElmt *idx = dlist_head(textures);
    float width = SCREEN_WIDTH - CARD_DIM;

    float widx = 0;
    while (idx != selected)
    {
        widx += CARD_DIM;
        idx = idx->next;
    }
    if (widx + CARD_DIM > width)
    {
        widx = -(widx + CARD_DIM - width);
    }
    else
    {
        widx = 0;
    }
    idx = dlist_head(textures);
    while (idx != NULL && widx + CARD_DIM <= width)
    {
        DrawRectangleLinesEx((Rectangle){
                                 .x = widx, .y = SCREEN_HEIGHT - CARD_DIM, .width = CARD_DIM, .height = CARD_DIM},
                             idx == selected ? 2.0f : 1.0f, idx == selected ? GREEN : BLACK);
        Texture2D out = ((SpriteSheet *)idx->data)->t;
        float maxdimScale = CARD_DIM / fmaxf(out.width, out.height);
        DrawTexturePro(out, (Rectangle){
                                .x = 0,
                                .y = 0,
                                .width = out.width,
                                .height = out.height,
                            },
                       (Rectangle){
                           .x = widx + CARD_DIM / 2 - (out.width * maxdimScale / 2) + EL_PADDING,
                           .y = SCREEN_HEIGHT - CARD_DIM / 2 - (out.height * maxdimScale / 2) + EL_PADDING,
                           .width = out.width * maxdimScale - EL_PADDING * 2,
                           .height = out.height * maxdimScale - EL_PADDING * 2,
                       },
                       (Vector2){0}, 0.0f, WHITE);
        idx = idx->next;
        widx += CARD_DIM;
    }
}

static void SaveMap(Rectangle world, List *levelObjList)
{
    const char *filename = "Map.bin";
    FILE *out = fopen(filename, "w");
    if (out == NULL)
    {
        fprintf(stderr, "ERROR: Failed to open file %s\n", filename);
        return;
    }
    printf("Saving %s\n", filename);
    fwrite(&world, sizeof(Rectangle), 1, out);
    fwrite(&levelObjList->size, sizeof(int), 1, out);
    ListElmt *objEl = list_head(levelObjList);
    while (objEl != NULL)
    {
        Object *obj = (Object *)objEl->data;
        size_t namelen = strlen(obj->name);
        fwrite(&namelen, sizeof(size_t), 1, out);
        fwrite(obj->name, namelen, 1, out);
        fwrite(&obj->type, sizeof(int), 1, out);
        fwrite(&obj->position, sizeof(Vector2), 1, out);
        fwrite(&obj->rotation, sizeof(float), 1, out);
        fwrite(&obj->scale, sizeof(float), 1, out);
        fwrite(&obj->sprites->size, sizeof(int), 1, out);
        DListElmt *spriteEl = dlist_head(obj->sprites);
        while (spriteEl != NULL)
        {
            SpriteRect *sprite = (SpriteRect *)spriteEl->data;
            size_t sprtnamelen = strlen(sprite->name);
            size_t shtnamelen = strlen(sprite->filename);
            fwrite(&sprtnamelen, sizeof(size_t), 1, out);
            fwrite(sprite->name, sprtnamelen, 1, out);
            fwrite(&shtnamelen, sizeof(size_t), 1, out);
            fwrite(sprite->filename, shtnamelen, 1, out);
            fwrite(&sprite->frameCount, sizeof(int), 1, out);
            fwrite(&sprite->r, sizeof(Rectangle), 1, out);
            fwrite(&sprite->action, sizeof(int), 1, out);
            fwrite(&sprite->colliders->size, sizeof(int), 1, out);
            ListElmt *colliderEl = list_head(sprite->colliders);
            while (colliderEl != NULL)
            {
                Collider *collider = (Collider *)colliderEl->data;
                fwrite(&collider->type, sizeof(int), 1, out);
                if (collider->type == RectType)
                {
                    fwrite(&collider->data.rect, sizeof(Rectangle), 1, out);
                }
                if (collider->type == LineType)
                {
                    fwrite(&collider->data.line, sizeof(Line), 1, out);
                }
                colliderEl = colliderEl->next;
            }
            spriteEl = spriteEl->next;
        }
        objEl = objEl->next;
    }
    fclose(out);
    printf("Saved %s\n", filename);
}

static void SelectionCycle(DList *list, DListElmt **current, int key)
{
    if (!list || list->size <= 0)
    {
        return;
    }
    if (key == KEY_PAGE_UP || key == KEY_HOME)
    {
        *current = dlist_is_head(*current) ? dlist_tail(list) : (*current)->prev;
    }
    else
    {
        *current = dlist_is_tail(*current) ? dlist_head(list) : (*current)->next;
    }
}