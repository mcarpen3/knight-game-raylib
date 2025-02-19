#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../datastructs/linkedlist.h"
#include "../datastructs/doublelinkedlist.h"
#include "../util/AutoGetSpriteRect.c"

#define MAX_INPUT_CHARS 16
#define ARRAY_LEN(x) (sizeof(x) / sizeof((x)[0]))
#define SHEET_PAD 8
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 800
#define WORLD_WIDTH (SCREEN_WIDTH) * (4)
#define WORLD_HEIGHT SCREEN_HEIGHT
#define SCALE_FACTOR 0.2f
#define ROT_FACTOR 2.0f
#define MAX_ZOOM 16.0f
#define MIN_ZOOM 0.5f
#define EL_PADDING 6.0f
#define FONTLG 30.0f
#define FONTSM 20.0f

typedef struct SpriteSheet
{
    char *filename;
    char *basename;
    DList *sheetRects;
    Texture t;
    Vector2 offset;
    DListElmt *selectedRect;
} SpriteSheet;

typedef struct SpriteRect
{
    char *name;
    char *filename;
    Rectangle r;
    Texture2D texture;
    int frameCount;
    int frameIdx;
    int frameCounter;
    List *colliders;
} SpriteRect;

typedef struct Object_ {
    char *name;
    float scale;
    float rotation;
    Vector2 position;
    DList *sprites;
    DListElmt *curSpriteEl;
} Object;

typedef struct Button_ {
    Vector2 position;
    bool clicked;
    bool disabled;
    bool hover;
    const char *text;
    Vector2 padding;
    Rectangle destination;
    float fontSize;
} Button;

void DestroySpriteRect(SpriteRect *sr);
void DestroySheet(SpriteSheet *ss);
void DestroyColliderRect(Rectangle *crect);
void DestroyObject(Object *obj);
static void DrawSavedSprites(SpriteSheet *sheet);
static void DrawSavedObjects(DList *objects, DListElmt *selected);
static void DrawObjectSprites(Object *obj);
static void SelfDestruct(DList *sheetList, List *level, DList *objects);
static void PlaceObjectInWorld(List *placedObjects, Object *obj);

static void DrawObject(Object *obj);
static void RectInput(Rectangle *rect, Vector2 mouseWorldPos, SpriteSheet *curSheet);
static void UpdateMode(int *mode);
static bool GetTextInput(char *input, bool *clearBuffer, int *charCount);
static void GetNumInput(int *input);
static void StoreSpriteRect(SpriteSheet *curSheet, Rectangle r, char *name, int frameCount);
static void InitSpriteSheets(const char *folder, DList *sheetList);
static Button InitButton(const char* text, Vector2 position, float fontSize);
static void DrawButton(Button b);
static void UpdateSpriteFrame(SpriteRect *sprite);


int main(void)
{
    const char *spritesFolder = "/home/matt/Documents/c/raylib/spritesheets";
    DList *sheetList = (DList *)malloc(sizeof(DList));
    List *levelObjs = (List *)malloc(sizeof(List));
    DList *objects = (DList *)malloc(sizeof(DList));
    char *input = (char *)malloc(128);
    char *name = (char *)malloc(MAX_INPUT_CHARS);
    char *info = (char *)malloc(256);
    char *frameCountStr = (char *)malloc(10 * sizeof(char));
    char *inputPrompt = "ENTER NAME: ";
    char *framesPrompt = "FRAMES: ";
    bool displayHelp = true;
    int letterCount = 0;
    bool textInputMode = false;
    bool numInputMode = false;
    bool clearInput = false;
    float panSpeed = 10.0f;
    int frameCount = 1;
    bool saving = false;
    int mode = 0;
    char *help[][6] = {
        {
            // mode0 - cut out sprite rects
            "'LEFTCLICK/DRAG'=Cut Out Rect",
            "'R'=Name Rect",
            "'ENTER'=Store Rect.",
            "'PGUP/PGDN'=Change Sheet",
            "'HOME/END'=Select Stored Rect.",
            "'DEL'=Delete Selected Rect.",
        },
        {
            // mode1 - draw colliders on sprite rects
            "'LEFTCLICK/DRAG'=Draw collider rects.",
            "'R'=Store Collider on Sprite",
            "'PGUP/PGDN'=Change Sheet",
            "'HOME/END'=Change Sprite",
        },
        {
            // mode2 - create objects from 1 or more spriteRects
            "'N'=Create New Object",
            "'R'=Add current sprite rect to the current object",
            "'PGUP/PGDN'=Change Sheet",
            "'HOME/END'=Change Sprite",
            "'UP/DOWN'=Change Object",
        },
        {
            // mode3 - place objects in the map
            "'UP/DOWN'=Scale",
            "'RIGHT/LEFT'=Rotate",
            "'LEFTCLICK'=Place Object",
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

    dlist_init(sheetList, (void *)DestroySheet);
    dlist_init(objects, (void *)DestroyObject);
    list_init(levelObjs, (void *)DestroyObject);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "MAP SPRITE EDITOR!");
    SetTargetFPS(60);
    InitSpriteSheets(spritesFolder, sheetList);

    Vector2 fontvec = MeasureTextEx(GetFontDefault(), "M", FONTLG, 1);
    DListElmt *curSheetElmt = sheetList->head;
    DListElmt *curObjElmt = objects->head;
    Camera2D camera = {0};
    Vector2 center = (Vector2){SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
    Rectangle r = (Rectangle){0};
    Button autoBtn = InitButton("CreateColliders", (Vector2) {
        .x = SCREEN_WIDTH / 2,
        .y = SCREEN_HEIGHT,
    }, FONTLG);

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
        SpriteSheet *curSheet = (SpriteSheet *)curSheetElmt->data;
        if (numInputMode) {
            GetNumInput(&frameCount);
            sprintf(frameCountStr, "%s%d", framesPrompt, frameCount);
        }
        if (textInputMode)
        {
            if (GetTextInput(name, &clearInput, &letterCount) == true) {
                textInputMode = false;
                if (mode == 0) {
                    StoreSpriteRect(curSheet, r, name, frameCount);
                    numInputMode = false;
                }
                if (mode == 2) {
                    Object *newObj = (Object *)malloc(sizeof(Object));
                    newObj->sprites = (DList *)malloc(sizeof(DList));
                    dlist_init(newObj->sprites, NULL);
                    newObj->position = (Vector2){0};
                    newObj->rotation = 0.0f;
                    newObj->scale = 1.0f;
                    newObj->name = strdup(name);
                    newObj->curSpriteEl = NULL;
                    dlist_ins_next(objects, dlist_tail(objects), (void *)newObj);
                    curObjElmt = dlist_tail(objects);
                }
                memset(name, 0, MAX_INPUT_CHARS);
                letterCount = 0;
                frameCount = 1;
            }
            sprintf(input, "%s%s", inputPrompt, name);
        }
        else
        {
            UpdateMode(&mode);
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
            if (IsKeyPressed(KEY_PAGE_UP))
            {
                curSheetElmt = dlist_is_tail(curSheetElmt) ? sheetList->head : curSheetElmt->next;
                r.width = 0.0f;
                r.height = 0.0f;
                r.x = 0.0f;
                r.y = 0.0f;
            }
            if (IsKeyPressed(KEY_PAGE_DOWN))
            {
                curSheetElmt = dlist_is_head(curSheetElmt) ? sheetList->tail : curSheetElmt->prev;
                r.width = 0.0f;
                r.height = 0.0f;
                r.x = 0.0f;
                r.y = 0.0f;
            }
            if (IsKeyPressed(KEY_HOME))
            {
                if (curSheet->sheetRects->size > 0)
                {
                    curSheet->selectedRect = dlist_is_tail(curSheet->selectedRect) ? curSheet->sheetRects->head : curSheet->selectedRect->next;
                }
            }
            if (IsKeyPressed(KEY_END))
            {
                if (curSheet->sheetRects->size > 0)
                {
                    curSheet->selectedRect = dlist_is_head(curSheet->selectedRect) ? curSheet->sheetRects->tail : curSheet->selectedRect->prev;
                }
            }
            if (IsKeyPressed(KEY_H))
            {
                displayHelp = !displayHelp;
            }

            if (mode == 0)
            {
                // sprite slice mode
                sprintf(info, "MODE: %s, File: \"%s\", Zoom: [%.2f]", modeStrs[mode], curSheet->basename, camera.zoom);
                RectInput(&r, mouseWorldPos, curSheet);
                if (IsKeyPressed(KEY_R))
                {
                    textInputMode = true;
                    numInputMode = true;
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
                // sprite colliders mode
                sprintf(info, "MODE: %s, File: \"%s\", Zoom: [%.2f]", modeStrs[mode], curSheet->basename, camera.zoom);
                if (curSheet->selectedRect == NULL) {
                    autoBtn.disabled = true;
                }
                RectInput(&r, mouseWorldPos, curSheet);
                if (IsKeyPressed(KEY_R) && curSheet->selectedRect != NULL)
                {
                    printf("INFO: Saving collider rect\n");
                    SpriteRect *sptr = (SpriteRect *)curSheet->selectedRect->data;
                    Rectangle *tmpRect = (Rectangle *)malloc(sizeof(Rectangle));
                    *tmpRect = (Rectangle) {
                        .x = r.x - curSheet->offset.x,
                        .y = r.y - curSheet->offset.y,
                        .width = r.width,
                        .height = r.height,
                    };
                    list_ins_next(sptr->colliders, list_tail(sptr->colliders), (void *)tmpRect);
                }
                if (CheckCollisionPointRec(GetMousePosition(), autoBtn.destination)) {
                    autoBtn.hover = true;
                    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && autoBtn.disabled == false) {
                        SpriteRect *sptr = (SpriteRect *)curSheet->selectedRect->data;
                        list_destroy(sptr->colliders);
                        list_init(sptr->colliders, (void *)DestroyColliderRect);
                        GetSpriteBounds(LoadImageFromTexture(sptr->texture), sptr->colliders);
                        printf("INFO: auto bounds found %d rects\n", sptr->colliders->size);
                    }
                } else {
                    autoBtn.hover = false;
                }
            }
            if (mode == 2)
            {
                // Object making mode
                sprintf(info, "MODE: %s, File: \"%s\", Zoom: [%.2f]", modeStrs[mode], curSheet->basename, camera.zoom);
                if (IsKeyPressed(KEY_N)) {
                    textInputMode = true;
                    clearInput = true;
                }
                if (IsKeyPressed(KEY_R)) {
                    // add the current spriteRect to the object
                    if (curSheet->selectedRect != NULL && curObjElmt != NULL) {
                        Object *curObj = (Object *)curObjElmt->data;
                        SpriteRect *curRect = (SpriteRect *)curSheet->selectedRect->data;
                        curObj->curSpriteEl = curSheet->selectedRect;
                        dlist_ins_next(curObj->sprites, dlist_tail(curObj->sprites), curRect);
                    }
                }
                if (IsKeyPressed(KEY_DOWN)) {
                    if (dlist_is_tail(curObjElmt)) {
                        curObjElmt = dlist_head(objects);
                    } else {
                        curObjElmt = curObjElmt->next;
                    }
                }
                if (IsKeyPressed(KEY_UP)) {
                    if (dlist_is_head(curObjElmt)) {
                        curObjElmt = dlist_tail(objects);
                    } else {
                        curObjElmt = curObjElmt->prev;
                    }
                }
            }
            if (mode == 3)
            {
                // object place mode
                sprintf(info, "MODE: %s, ObjName: \"%s\", Zoom: [%.2f]", modeStrs[mode],
                    curObjElmt == NULL ? "No objects" : ((Object *)curObjElmt->data)->name,
                    camera.zoom); 
                if (curObjElmt != NULL)
                {
                    Object *curObj = (Object *)curObjElmt->data;
                    DListElmt *placedObject = dlist_head(objects);
                    while (placedObject != NULL) {
                        Object *tmpobj = (Object *)placedObject->data;
                        if (tmpobj->curSpriteEl != NULL) {
                            SpriteRect *tmprect = (SpriteRect *)tmpobj->curSpriteEl->data;
                            UpdateSpriteFrame(tmprect);
                        }
                        placedObject = placedObject->next;
                    }                    
                    if (curObj->sprites->size > 0) {
                        curObj->position = mouseWorldPos;
                        SpriteRect *rect = (SpriteRect *)curObj->curSpriteEl;
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
                            PlaceObjectInWorld(levelObjs, curObj);
                        }
                        // if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
                        // {
                        //     // drag the mouse to place more sprites adjacent to the currently placed one
                        //     ListElmt *last = list_head(levelObjs);
                        //     Object *placedObj = (Object *)last->data;
                        //     SpriteRect *placed = (SpriteRect *)list_head(placedObj->sprites);
                        //     if (!CheckCollisionRecs((Rectangle){
                        //                                 .width = rect->r.width * curObj->scale,
                        //                                 .height = rect->r.height * curObj->scale,
                        //                                 .x = curObj->position.x - (rect->r.width * curObj->scale / 2),
                        //                                 .y = curObj->position.y - (rect->r.height * curObj->scale / 2),
                        //                             },
                        //                             (Rectangle) {
                        //                                 .width = placed->r.width * placedObj->scale,
                        //                                 .height = placed->r.height * placedObj->scale,
                        //                                 .x = placedObj->position.x - (placed->r.width * placedObj->scale / 2),
                        //                                 .y = placedObj->position.y - (placed->r.height * placedObj->scale / 2)
                        //                             }))
                        //     {
                        //         PlaceObjectInWorld(levelObjs, curObj);
                        //     }
                        // }
                        if (IsKeyPressed(KEY_F) && levelObjs->size > 0)
                        {
                            
                        }
                        if (IsKeyPressed(KEY_K))
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
            if (curSheet->selectedRect != NULL)
            {
                SpriteRect *tmprect = (SpriteRect *)curSheet->selectedRect->data;
                Rectangle destination = (Rectangle){
                    .x = (SCREEN_WIDTH / 2) - (tmprect->r.width / 2),
                    .y = (SCREEN_HEIGHT / 2) - (tmprect->r.height / 2),
                    .width = tmprect->r.width,
                    .height = tmprect->r.height};
                DrawTexturePro(tmprect->texture, tmprect->r, destination, (Vector2){0}, 0.0f, WHITE);
                DrawRectangleLinesEx(destination, 1.0f, BLACK);
                ListElmt *i = tmprect->colliders->head;
                while (i != NULL)
                {
                    Rectangle *tmp = (Rectangle *)i->data;
                    Rectangle out = (Rectangle) {
                        .x = destination.x + tmp->x,
                        .y = destination.y + tmp->y,
                        .width = tmp->width,
                        .height= tmp->height,
                    };
                    DrawRectangleLinesEx(out, 1.0f, GREEN);
                    i = i->next;
                }
            }
            DrawRectangleLinesEx(r, 1.0f, GREEN);
        }
        if (mode == 2)
        {
            // Object collection mode
        }
        if (mode == 3)
        {
            // draw the world rectangle
            DrawRectangle(0.0f, 0.0f, WORLD_WIDTH, WORLD_HEIGHT, BROWN);
            // draw the saved objects
            ListElmt *levelEl = levelObjs->head;
            while (levelEl != NULL)
            {
                Object *curObj = (Object *)levelEl->data;
                DrawObject(curObj);
                levelEl = levelEl->next;
            }
            if (curObjElmt != NULL) {
                Object *curObj = (Object *)curObjElmt->data;
                DrawObject(curObj);
            }
        }
        EndMode2D();
        // Draw Crosshairs
        DrawLine(SCREEN_WIDTH / 2, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT, BLACK);
        DrawLine(0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2, BLACK);
        if (saving)
        {
            DrawText("Saving...", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, FONTLG, BLUE);
        }
        if (mode == 1 && !autoBtn.disabled) {
            DrawButton(autoBtn);
        }
        if (textInputMode)
        {
            Rectangle input1 = (Rectangle){
                .x = 4,
                .y = SCREEN_HEIGHT - FONTLG,
                .width = (strlen(inputPrompt) + MAX_INPUT_CHARS) * fontvec.x,
                .height = FONTLG + 2,
            };
            DrawText(input, input1.x + 2, input1.y + 2, FONTLG, WHITE);
            DrawRectangleLines(input1.x, input1.y, input1.width, input1.height, GREEN);
            if (numInputMode) {
                Rectangle input2 = (Rectangle){
                    .x = input1.width + 2,
                    .y = input1.y,
                    .width = fontvec.x * strlen(frameCountStr),
                    .height = input1.height,
                };
                DrawText(frameCountStr, input2.x + 2, input2.y, FONTLG, WHITE);
                DrawRectangleLines(input2.x, input2.y, input2.width, input2.height, GREEN);
            }
        }
        DrawText(info, 4, 0, FONTLG, GOLD);
        if (displayHelp)
        {
            float curse = 0;
            for (int i = 0; i < ARRAY_LEN(help[mode]); ++i)
            {
                curse = 40 + (i * (FONTLG + 10));
                if (help[mode][i] != NULL)
                    DrawText(help[mode][i], 4, curse, FONTLG, RAYWHITE);
            }
            curse += FONTLG + 10;
            for (int i = 0; i < ARRAY_LEN(commonHelp); ++i)
            {
                DrawText(commonHelp[i], 4, curse + (i * (FONTLG + 10)), FONTLG, RAYWHITE);
            }
        }
        if (mode == 0 || mode == 1 || mode == 2) {
            DrawSavedSprites(curSheet);
        }
        if (mode == 2) {
            if (curObjElmt != NULL) {
                Object *curObj = (Object *)curObjElmt->data;
                DrawObjectSprites(curObj);
            }
        }
        if (mode == 2 || mode == 3) {
            DrawSavedObjects(objects, curObjElmt);
        }
        EndDrawing();
    }
    SelfDestruct(sheetList, levelObjs, objects);
    free(info);
    free(frameCountStr);
}

void DestroySpriteRect(SpriteRect *sr)
{
    printf("INFO: SpriteRect %s destroying\n", sr->name);
    free(sr->filename);
    free(sr->name);
    free(sr);
}

void DestroySheet(SpriteSheet *ss)
{
    printf("INFO: SpriteSheet %s destroying\n", ss->filename);
    UnloadTexture(ss->t);
    free(ss->filename);
    free(ss->basename);
    free(ss);
}

void DestroyColliderRect(Rectangle *rect)
{
    free(rect);
}

void DestroyObject(Object *obj) {
    free(obj->name);
}

static void DrawSavedObjects(DList *objects, DListElmt *selected) {
    float yidx = 0.0f;
    float objFontSz = 20.0f;
    float rmargin = 166.0f;
    Vector2 objFontPad = {.x = EL_PADDING, .y = EL_PADDING};
    if (objects->size == 0) {
        const char *nomsg = "No objects saved";
        Vector2 fontvec = MeasureTextEx(GetFontDefault(), nomsg, 20.0f, 2.0f);
        DrawText("No objects saved", GetScreenWidth() - rmargin - fontvec.x - objFontPad.x, yidx, 20, WHITE);
    }
    DListElmt *cursor = dlist_head(objects);
    while(cursor != NULL) {
        bool sel = selected ? true : false;
        Object *obj = (Object *)cursor->data;
        char *output = (char *)malloc(64);
        sprintf(output, "%s +%d", obj->name, obj->sprites->size);
        Vector2 fontvec = MeasureTextEx(GetFontDefault(), output, objFontSz, 2.0f);
        Rectangle box = (Rectangle) {
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

static void DrawObjectSprites(Object *obj) {
    float yidx2 = 30.0f;
    DListElmt *el = dlist_head(obj->sprites);
    while(el != NULL) {
        SpriteRect *sprite = (SpriteRect *)el->data;
        Rectangle dest = (Rectangle) {
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

static void DrawSavedSprites(SpriteSheet *sheet)
{
    Vector2 fontvec = MeasureTextEx(GetFontDefault(), "M", 20.0f, 1.0f);
    float rmargin = 160.0f;
    float yidx = 0.0f;
    if (sheet->sheetRects->size == 0)
    {
        DrawText("No rects saved", GetScreenWidth() - rmargin, yidx, 20, WHITE);
        return;
    }
    DListElmt *el = sheet->sheetRects->head;

    while (el != NULL)
    {
        bool sel = el == sheet->selectedRect ? true : false;
        SpriteRect *sr = (SpriteRect *)el->data;
        float xscale = rmargin / sr->r.width;
        Rectangle src = (Rectangle){
            .x = sr->r.x,
            .y = sr->r.y,
            .width = sr->r.width,
            .height = sr->r.height,
        };
        Rectangle dest = (Rectangle){
            .x = SCREEN_WIDTH - rmargin,
            .y = yidx,
            .width = rmargin,
            .height = sr->r.height * xscale < fontvec.y ? fontvec.y : sr->r.height * xscale,
        };
        DrawTexturePro(sheet->t, src, dest, (Vector2){0.0f, 0.0f}, 0.0f, WHITE);
        DrawRectangleLinesEx(dest, 
            sel ? 2.0f : 1.0f, sel ? GREEN : BLACK);
        DrawText(sr->name, dest.x + EL_PADDING, dest.y, 20, WHITE);
        yidx += sr->r.height * xscale;
        el = el->next;
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
    objcpy->rotation = obj->rotation;
    objcpy->sprites = obj->sprites;
    objcpy->curSpriteEl = obj->curSpriteEl;
    list_ins_next(placedObjects, list_tail(placedObjects), objcpy);
}

static void UpdateSpriteFrame(SpriteRect *sprite) {
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
    if (obj->curSpriteEl == NULL) {
        return;
    }
    SpriteRect *sprite = (SpriteRect *)obj->curSpriteEl->data;

    Rectangle destination = (Rectangle){
        .width = sprite->r.width * obj->scale / sprite->frameCount,
        .height = sprite->r.height * obj->scale,
        .x = Clamp(obj->position.x, (sprite->r.width * obj->scale / 2) / sprite->frameCount,
                   WORLD_WIDTH - (sprite->r.width * obj->scale / 2) / sprite->frameCount),
        .y = Clamp(obj->position.y, sprite->r.height * obj->scale / 2,
                   WORLD_HEIGHT - sprite->r.height * obj->scale / 2)};

    Vector2 origin = (Vector2){
        destination.width / 2,
        destination.height / 2};

    Rectangle dr = (Rectangle){
        .x = sprite->r.x + (sprite->r.width / sprite->frameCount * sprite->frameIdx),
        .y = sprite->r.y,
        .width = sprite->r.width / sprite->frameCount,
        .height = sprite->r.height,
    };
    DrawTexturePro(sprite->texture, dr, destination, origin, obj->rotation, WHITE);
}

static void RectInput(Rectangle *r, Vector2 mouseWorldPos, SpriteSheet *curSheet)
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        r->x = Clamp(mouseWorldPos.x, curSheet->offset.x, curSheet->offset.x + curSheet->t.width);
        r->y = Clamp(mouseWorldPos.y, curSheet->offset.y, curSheet->offset.y + curSheet->t.height);
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        r->width = Clamp(mouseWorldPos.x - r->x, 0.0f, curSheet->t.width - (r->x - curSheet->offset.x));
        r->height = Clamp(mouseWorldPos.y - r->y, 0.0f, curSheet->t.height - (r->y - curSheet->offset.y));
    }
}

static void UpdateMode(int *mode)
{
    if (IsKeyPressed(KEY_ONE))
    {
        *mode = 0;
    }
    if (IsKeyPressed(KEY_TWO))
    {
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

static void GetNumInput(int *input)
{
    if (IsKeyPressed(KEY_UP))
    {
        *input = Clamp(*input + 1, 1, 60);
    }
    if (IsKeyPressed(KEY_DOWN))
    {
        *input = Clamp(*input - 1, 1, 60);
    }
}
static void StoreSpriteRect(SpriteSheet *curSheet, Rectangle r, char *name, int frameCount) {

    SpriteRect *newRect = (SpriteRect *)malloc(sizeof(SpriteRect));
    newRect->colliders = (List *)malloc(sizeof(List));
    list_init(newRect->colliders, (void *)DestroyColliderRect);
    newRect->r = (Rectangle) {
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
    dlist_ins_next(curSheet->sheetRects, dlist_tail(curSheet->sheetRects), newRect);
    curSheet->selectedRect = dlist_tail(curSheet->sheetRects);
}

static void InitSpriteSheets(const char *folder, DList *sheetList) {
    FilePathList spritesheets = LoadDirectoryFilesEx(folder, ".png", true);
    // initialize all the spritesheets
    for (int i = 0; i < spritesheets.count; ++i)
    {
        if (strcmp(GetFileExtension(spritesheets.paths[i]), ".png"))
            continue;
        SpriteSheet *sheet = (SpriteSheet *)malloc(sizeof(SpriteSheet));
        sheet->filename = strdup(spritesheets.paths[i]);
        sheet->basename = strdup(GetFileName(spritesheets.paths[i]));
        sheet->t = LoadTexture(spritesheets.paths[i]);
        sheet->selectedRect = NULL;
        DList *rectlist = (DList *)malloc(sizeof(DList));
        dlist_init(rectlist, (void *)DestroySpriteRect);
        sheet->sheetRects = rectlist;
        sheet->offset = (Vector2){
            SCREEN_WIDTH / 2 - sheet->t.width / 2,
            SCREEN_HEIGHT / 2 - sheet->t.height / 2};
        dlist_ins_next(sheetList, dlist_tail(sheetList), sheet);
    }
}

static Button InitButton(const char* text, Vector2 position, float fontSize) {
    Vector2 fontvec = MeasureTextEx(GetFontDefault(), text, fontSize, 1);
    Button b = (Button) {
        .text = text,
        .clicked = false,
        .hover = false,
        .disabled = false,
        .fontSize = fontSize,
        .padding = (Vector2){.x = EL_PADDING, .y = EL_PADDING},
        .position = position,
    };
    b.destination = (Rectangle) {
        .x = position.x - (fontvec.x / 2 - b.padding.x),
        .y = position.y - (fontvec.y + b.padding.y * 2),
        .width = fontvec.x + (b.padding.x * 2),
        .height = fontvec.y + (b.padding.y * 2)
    };
    return b;
}

static void DrawButton(Button b) {
    DrawRectangleLinesEx(b.destination, 2.0f, WHITE);
    Vector2 shift = (Vector2) {
        .x = b.destination.x + b.padding.x,
        .y = b.destination.y + b.padding.y,
    };
    if (b.hover) {
        DrawRectangleRec(b.destination, WHITE);
        DrawTextPro(GetFontDefault(), b.text, shift, (Vector2){0, 0}, 0.0f, b.fontSize, 1.0f, BLACK);
    } else {
        DrawTextPro(GetFontDefault(), b.text, shift, (Vector2){0, 0}, 0.0f, b.fontSize, 1.0f, WHITE);
    }
}