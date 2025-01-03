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
    const int screenWidth = 1920;
    const int screenHeight = 1080;
    char *info = (char *)malloc(256);

    InitWindow(screenWidth, screenHeight, "raylib [texture] example - sprite anim");
    // NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)
    Object *o = ObjectFactory("knight", (Vector2){(float)screenWidth/2, (float)screenHeight/2});
    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyDown(KEY_RIGHT)) Forward(o);
        if (IsKeyDown(KEY_LEFT)) Backward(o);
        if (IsKeyUp(KEY_RIGHT) && IsKeyUp(KEY_LEFT)) Decel(o);
        if (IsKeyDown(KEY_SPACE)) Jump(o);
        if (IsKeyUp(KEY_SPACE)) Land(o);
        Update(o);
        sprintf(info, "x: %.3f | y: %.3f | vx: %.3f | vy: %.3f | rev: %s | jmp %d", 
            o->pos.x, o->pos.y, o->vel.x, o->vel.y, o->sprite->rev ? "true" : "false", o->jc);
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            DrawText(info, 0, 0, 30, WHITE);
            ClearBackground(DARKBLUE);
            Draw(o);
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