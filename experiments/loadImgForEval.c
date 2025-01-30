
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../datastructs/linkedlist.h"

#define MAX_INPUT_CHARS 16
#define ARRAY_LEN(x)(sizeof(x) / sizeof((x)[0]))
#define SHEET_PAD       8

typedef struct SpriteSheet {
    Vector2 *offset;
    char *filename;
    List *sheetRects;
    Texture *t;
} SpriteSheet;

typedef struct SpriteRect
{
    char *name;
    Rectangle *r;
} SpriteRect;

void DestroySpriteRect(SpriteRect *sr);
void DestroySheet(SpriteSheet *ss);
static void DisplaySavedRects(List *rectlist, Texture t, Vector2 origin, ListElmt *selected);

int main(void)
{
    const char *spritesheet = "../spritesheets/background";
    FilePathList spritesheets = LoadDirectoryFiles(spritesheet);
    Vector2 textureoffset = (Vector2){0};
    List *sheetList = (List *)malloc(sizeof(List));
    list_init(sheetList, (void *)DestroySheet);
    bool namingSprite = false;
    bool clearInput = false;
    const int screenWidth = 1920;
    const int screenHeight = 1080;
    float panSpeed = 5.0f;
    char *info = (char *)malloc(256);
    char *instructions[] = {
        "'MOUSESCROLL'=zoom", 
        "'LEFTCLICK/DRAG'=draw", 
        "'R'=name rect",
        "'ENTER'=save rect",
        "'F'=save sheet",
        "'W/A/S/D'=pan",
        "'PGDN'=Select Saved Rect."
    };
    // char *stats = (char *)malloc(256);

    InitWindow(screenWidth, screenHeight, "SpriteSheetSlicer!");
    SetTargetFPS(60);
    // initialize all the spritesheets
    float ymax = 0;
    for (int i = 0; i < spritesheets.count; ++i) {
        if (strcmp(GetFileExtension(spritesheets.paths[i]), ".png")) continue;
        SpriteSheet *sheet = (SpriteSheet *)malloc(sizeof(SpriteSheet));
        sheet->filename = strdup(spritesheets.paths[i]);
        Texture2D t = LoadTexture(spritesheets.paths[i]);
        if (t.height > ymax) ymax = t.height;
        sheet->offset = (Vector2 *)malloc(sizeof(Vector2));
        if (textureoffset.x >= screenWidth) {
            textureoffset.x = 0.0f;
            textureoffset.y += ymax;
        }
        sheet->offset->x = textureoffset.x;
        sheet->offset->y = textureoffset.y;
        textureoffset.x += t.width;
        sheet->t = (Texture2D *)malloc(sizeof(Texture2D));
        *sheet->t = t;
        List *rectlist = (List *)malloc(sizeof(List));
        list_init(rectlist, (void *)DestroySpriteRect);
        sheet->sheetRects = rectlist;
        list_ins_next(sheetList, NULL, sheet);
    }
    ListElmt *c = sheetList->head;
    while (c != NULL) {
        SpriteSheet *cur = (SpriteSheet *)c->data;  
        printf("filename: %s, width: %d, height: %d\n", cur->filename, cur->t->width, cur->t->height);
        c = c->next;
    }
    Camera2D camera = {0};
    Vector2 center = (Vector2){screenWidth / 2, screenHeight / 2};
    Rectangle r = (Rectangle){.x = 0.0f, .y = 0.0f, .width = 0.0f, .height = 0.0f};
    camera.target = (Vector2){center.x, center.y};
    camera.offset = (Vector2){center.x, center.y};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
    // user input
    int letterCount = 0;
    char *input = (char *)malloc(128);
    char name[MAX_INPUT_CHARS + 1] = "\0";
    float fontSize = 30.0f;
    int key;
    ListElmt *cursor = sheetList->head;
    
    char *inputPrompt = "ENTER SPRITE NAME: ";

    while (!WindowShouldClose())
    {
        sprintf(info, "Sprite Rect. { x: %.2f, y: %.2f, w: %.2f, h: %.2f }, zoom: %f", 
            r.x, r.y, r.width, r.height, camera.zoom);
        camera.zoom = Clamp(camera.zoom + (float)GetMouseWheelMove() * 0.2f, 1.0f, 16.0f);

        // Get the world point that is under the mouse
        Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

        // Set the offset to where the mous}e is
        camera.offset = GetMousePosition();

        // Set the target to match, so that the camera maps the world space point
        // under the cursor to the screen space point under the cursor at any zoom
        camera.target = mouseWorldPos;

        if (namingSprite)
        {
            if (clearInput) {
                key = GetCharPressed();
                while (key > 0) {
                    key = GetCharPressed();
                }
                clearInput = false;
            }
            // if (IsKeyPressed(KEY_ENTER) && letterCount > 0)
            // {
            //     SpriteRect *sptr = (SpriteRect *)malloc(sizeof(SpriteRect));
            //     sptr->r = (Rectangle *)malloc(sizeof(Rectangle));
            //     namingSprite = false;
            //     sptr->name = strdup(name);
            //     sptr->r->x = r.x - textureoffset.x;
            //     sptr->r->y = r.y - textureoffset.y;
            //     sptr->r->width = r.width;
            //     sptr->r->height = r.height;
            //     list_ins_next(rectlist, NULL, sptr);
            //     name[0] = '\0';
            //     letterCount = 0;
            //     clearInput = false;
            //     continue;
            // }
            // Get char pressed (unicode character) on the queue
            key = GetCharPressed();
            // Check if more characters have been pressed on the same frame
            while (key > 0)
            {
                // NOTE: Only allow keys in range [32..125]
                if ((key >= 32) && (key <= 125) && (letterCount < MAX_INPUT_CHARS))
                {
                    name[letterCount] = (char)key;
                    name[letterCount + 1] = '\0'; // Add null terminator at the end of the string.
                    letterCount++;
                }
                key = GetCharPressed(); // Check next character in the queue
            }

            if (IsKeyPressed(KEY_BACKSPACE))
            {
                letterCount--;
                if (letterCount < 0)
                    letterCount = 0;
                name[letterCount] = '\0';
            }
            sprintf(input, "%s%s", inputPrompt, name);
        } else {

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
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                r.x = mouseWorldPos.x;
                r.y = mouseWorldPos.y;
            }
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            {
                r.width = mouseWorldPos.x - r.x;
                r.height = mouseWorldPos.y - r.y;
            }
            if (IsKeyPressed(KEY_R))
            {
                namingSprite = true;
                clearInput = true;
            }
            // if (IsKeyPressed(KEY_PAGE_DOWN) && rectlist->size > 0) {
            //     if (curElmt == NULL) {
            //         curElmt = rectlist->head;
            //     } else if (curElmt->next == NULL) {
            //         curElmt = rectlist->head;
            //     } else {
            //         curElmt = curElmt->next;
            //     }
            // }
            // if (IsKeyPressed(KEY_DELETE) && rectlist->size > 0) {
            //     ListElmt *tmp = rectlist->head;
            //     SpriteRect *tmpRect = (SpriteRect *)malloc(sizeof(SpriteRect));
            //     tmpRect->r = (Rectangle *)malloc(sizeof(Rectangle));
            //     tmpRect->name = (char *)malloc(64);
            //     if (tmp == curElmt) {
            //         list_rem_next(rectlist, NULL, (void *)&tmpRect);
            //     } else {
            //         while(tmp->next != curElmt) {
            //             tmp = tmp->next;
            //         }
            //         list_rem_next(rectlist, tmp, (void *)&tmpRect);
            //     }
            //     curElmt = rectlist->head;
            //     printf("deleted %s\n", tmpRect->name);
            //     free(tmpRect->name);
            //     free(tmpRect->r);
            //     free(tmpRect);
            // }
            // if (IsKeyPressed(KEY_F) && rectlist->size > 0) {
            //     char *output = (char*)malloc(128);
            //     sprintf(output, "%s/%s.atlas", GetDirectoryPath(spritesheet), GetFileNameWithoutExt(spritesheet));
            //     FILE *outfile = fopen(output, "w");
            //     ListElmt *tmpelmt = rectlist->head;
            //     while(tmpelmt != NULL) {
            //         SpriteRect *tmprect = (SpriteRect *)tmpelmt->data;
            //         size_t nameLength = strlen(tmprect->name);
            //         fwrite(&nameLength, nameLength, 1, outfile);
            //         fwrite(tmprect->name, nameLength, 1, outfile);
            //         fwrite(tmprect->r, sizeof(Rectangle), 1, outfile);
            //         tmpelmt = tmpelmt->next;
            //     }
            //     fclose(outfile);
            //     free(output);
            //     printf("Saved sprite atlas with %d rectangles to %s\n", rectlist->size, spritesheet);
            // } 
        }
        BeginDrawing();

        ClearBackground(DARKGRAY);
        BeginMode2D(camera);
        
        ListElmt *idx = sheetList->head;
        
        while(idx != NULL) {
            SpriteSheet *sheet = (SpriteSheet *)idx->data;
            DrawTexture(*sheet->t, sheet->offset->x, sheet->offset->y, RAYWHITE);
            idx = idx->next;
        }
        DrawRectangleLines(r.x, r.y, r.width, r.height, GREEN);
        ListElmt *tmp = sheetList->head;
        while (tmp != NULL) {
            SpriteSheet *tmpsheet = (SpriteSheet *)tmp->data;
            Rectangle tmprect = (Rectangle) {
                .x = tmpsheet->offset->x,
                .y = tmpsheet->offset->y,
                .width = tmpsheet->t->width,
                .height = tmpsheet->t->height
            };
            if (CheckCollisionPointRec(mouseWorldPos, tmprect)) {
                DrawRectangleLinesEx(tmprect, 1.0f, GREEN);
                break;
            }
            tmp = tmp->next;
        }
        // DrawRectangleLines(textureoffset.x, textureoffset.y, t.width, t.height, RED);
        EndMode2D();
        if (namingSprite) {
            DrawText(input, 4, GetScreenHeight() - fontSize, fontSize, WHITE);
            DrawRectangleLines(2, GetScreenHeight() - fontSize - 4, (strlen(inputPrompt) + MAX_INPUT_CHARS) * MeasureTextEx(GetFontDefault(), "M", fontSize,1).x, fontSize + 2, RED);
        }
        DrawText(info, 4, 0, 40, RAYWHITE);
        for (int i = 0; i < ARRAY_LEN(instructions); ++i) {
            DrawText(instructions[i], 4, 40 + (i * (fontSize + 10)), 40, RAYWHITE);
        }
        // DisplaySavedRects(rectlist, t, center, curElmt);
        EndDrawing();
    }

    free(info);
    cursor = sheetList->head;
    while(cursor != NULL) {
        SpriteSheet *spriteSheet = (SpriteSheet *)cursor->data;
        printf("destroying %s rects\n", spriteSheet->filename);
        list_destroy(spriteSheet);
        cursor = cursor->next;
    }
}

void DestroySpriteRect(SpriteRect *sr)
{
    printf("INFO: SpriteRect %s destroying\n", sr->name);
    free(sr->name);
    free(sr->r);
}

void DestroySheet(SpriteSheet *ss) {
    printf("INFO: SpriteSheet %s destroying\n", ss->filename);
    list_destroy(ss->sheetRects);
    UnloadTexture(*ss->t);
    free(ss->filename);
    free(ss->offset);
}

static void DisplaySavedRects(List *rectlist, Texture t, Vector2 origin, ListElmt *selected)
{
    float xmargin = 200.0f;
    float yidx = 0.0f;
    ListElmt *el = rectlist->head;

    while (el != NULL)
    {
        SpriteRect *sr = (SpriteRect *)el->data;
        float xscale = xmargin / sr->r->width;
        Rectangle src = (Rectangle){
            .x = sr->r->x,
            .y = sr->r->y,
            .width = sr->r->width,
            .height = sr->r->height,
        };
        Rectangle dest = (Rectangle){
            .x = GetScreenWidth() - xmargin,
            .y = yidx,
            .width = xmargin,
            .height = sr->r->height * xscale,
        };
        DrawTexturePro(t, src, dest, (Vector2){0.0f, 0.0f}, 0.0f, WHITE);
        if (el == selected) {
            DrawRectangleLinesEx(dest, 2.0f, GREEN);
        }
        DrawText(sr->name, dest.x, dest.y, 20, WHITE);
        yidx += sr->r->height * xscale;
        el = el->next;
    }
}