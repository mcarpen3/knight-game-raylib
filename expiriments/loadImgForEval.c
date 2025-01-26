
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../datastructs/linkedlist.h"

#define MAX_INPUT_CHARS 16

typedef struct SpriteRect
{
    char *name;
    Rectangle *r;
} SpriteRect;

void DestroySpriteRect(SpriteRect *sr);
static void DisplaySavedRects(List *rectlist, Texture t, Vector2 origin);

int main(void)
{
    const char *spritesheet = "../spritesheets/background/terrain.png";
    bool namingSprite = false;
    const int screenWidth = 1920;
    const int screenHeight = 1080;
    float panSpeed = 5.0f;
    char *info = (char *)malloc(256);
    char *instructions = "Scroll to zoom. LEFT click/drag: draw rectangle. 'R': save rect.";
    // char *stats = (char *)malloc(256);

    InitWindow(screenWidth, screenHeight, "SpriteSheetSlicer!");
    SetTargetFPS(60);
    Texture2D t = LoadTexture(spritesheet);
    Camera2D camera = {0};
    Vector2 center = (Vector2){screenWidth / 2, screenHeight / 2};
    Vector2 textureoffset = (Vector2){center.x - t.width / 2, center.y - t.height / 2};
    Rectangle r = (Rectangle){.x = 0.0f, .y = 0.0f, .width = 0.0f, .height = 0.0f};
    List *rectlist = (List *)malloc(sizeof(List));
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
    list_init(rectlist, (void *)DestroySpriteRect);

    while (!WindowShouldClose())
    {
        sprintf(info, "Rect. { x: %.2f, y: %.2f, w: %.2f, h: %.2f }", r.x, r.y, r.width, r.height);
        camera.zoom += ((float)GetMouseWheelMove() * 0.2f);
        // Get the world point that is under the mouse
        Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

        // Set the offset to where the mouse is
        camera.offset = GetMousePosition();

        // Set the target to match, so that the camera maps the world space point
        // under the cursor to the screen space point under the cursor at any zoom
        camera.target = mouseWorldPos;

        if (namingSprite)
        {
            if (IsKeyPressed(KEY_ENTER) && letterCount > 0)
            {
                SpriteRect *sptr = (SpriteRect *)malloc(sizeof(SpriteRect));
                sptr->r = (Rectangle *)malloc(sizeof(Rectangle));
                namingSprite = false;
                sptr->name = strdup(name);
                sptr->r->x = r.x - textureoffset.x;
                sptr->r->y = r.y - textureoffset.y;
                sptr->r->width = r.width;
                sptr->r->height = r.height;
                list_ins_next(rectlist, NULL, sptr);
                name[0] = '\0';
                letterCount = 0;
                continue;
            }
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
            sprintf(input, "INPUT: %s", name);
            DrawText(input, 4, GetScreenHeight() - fontSize, fontSize, WHITE);
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
            }
        }
        BeginDrawing();

        ClearBackground(DARKGRAY);
        BeginMode2D(camera);

        DrawTexture(t, textureoffset.x, textureoffset.y, RAYWHITE);
        DrawRectangleLines(r.x, r.y, r.width, r.height, GREEN);
        DrawRectangleLines(textureoffset.x, textureoffset.y, t.width, t.height, RED);
        EndMode2D();

        DrawText(info, 4, 0, 40, RAYWHITE);
        DrawText(instructions, 4, 40, 40, RAYWHITE);
        DisplaySavedRects(rectlist, t, center);
        EndDrawing();
    }
    UnloadTexture(t);
    free(info);
    list_destroy(rectlist);
}

void DestroySpriteRect(SpriteRect *sr)
{
    printf("INFO: SpriteRect %s destroying\n", sr->name);
    free(sr->name);
    free(sr->r);
}

static void DisplaySavedRects(List *rectlist, Texture t, Vector2 origin)
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
        DrawRectangleLinesEx(dest, 2.0f, GREEN);
        DrawText(sr->name, dest.x, dest.y, 20, WHITE);
        yidx += sr->r->height * xscale;
        el = el->next;
    }
}