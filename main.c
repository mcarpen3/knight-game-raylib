#include "raylib.h"
#include "./object/object.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define MAX_FRAME_SPEED     15
#define MIN_FRAME_SPEED      1
#define ARRAY_LEN(x)(sizeof(x) / sizeof((x)[0]))

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    srand(time(NULL));
    const int screenWidth = 3200;
    const int screenHeight = 1800;
    char *info = (char *)malloc(256);

    InitWindow(screenWidth, screenHeight, "Knight HERO game!!!!");
    // NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)
    Object *o = ObjectFactory("knight", (Vector2){(float)screenWidth/2, (float)screenHeight/2}, (Vector2){120.0f, 80.0f});
    Object *skeleton = ObjectFactory("skeleton", (Vector2){(float)screenWidth / 3, (float)screenHeight /2}, (Vector2){64.0f, 48.0f});
    Object *curObj = o;
    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyPressed(KEY_S)) {
            if (curObj == o) {
                curObj = skeleton;
            } else {
                curObj = o;
            }
        }
        if (IsKeyDown(KEY_LEFT_CONTROL)) {
            ScaleSprite(curObj);
        } else if (IsKeyDown(KEY_LEFT_SHIFT)) {
            ScaleCollider(curObj);
        } else if (IsKeyDown(KEY_LEFT_ALT)) {
            ShiftOffset(curObj);
        } else {
            if (IsKeyDown(KEY_RIGHT)) Forward(curObj);
            else if (IsKeyDown(KEY_LEFT)) Backward(curObj);
            else Decel(curObj);
            if (IsKeyDown(KEY_UP)) Jump(curObj);
            if (IsKeyDown(KEY_SPACE)) Attack(curObj);
            if (IsKeyUp(KEY_SPACE) && IsKeyUp(KEY_UP)) {
                Rest(curObj);
            }
        }

        Update(o);
        Update(skeleton);
        // sprintf(info, "posx: %.3f | posy: %.3f | recx: %.3f | recy: %.3f | recw: %.3f | recy: %.3f", 
        //     o->pos.x, o->pos.y, o->c.x, o->c.y, o->c.width, o->c.height);
        sprintf(info, "%s: scale: %.3f, scalex: %.3f, scaley: %.3f, offsetx: %.3f", 
            curObj->name, curObj->spriteScale, curObj->dscale.x, curObj->dscale.y, curObj->offsetX);
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(DARKBLUE);
            DrawText(info, 2, 60, 40, WHITE);
            DrawText("HOLD L_CTRL AND USE ARROWS TO SCALE SPRITE", 2, 60 + 40 + 2, 40, WHITE);
            DrawText("HOLD L_ALT AND USE ARROWS TO SCALE COLLIDER", 2, 60 + 40 * 2 + 2, 40, WHITE);
            DrawText("HOLD L_SHIFT AND USE ARROWS TO OFFSET COLLIDER", 2, 60 + 40 * 3 + 2, 40, WHITE);
            Draw(o);
            Draw(skeleton);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }
    // De-Initialization
    //--------------------------------------------------------------------------------------
    Destroy(o);
    CloseWindow();                // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    return 0;
}