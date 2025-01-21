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

    InitWindow(screenWidth, screenHeight, "Knight HERO game!!!!");
    float maxHeight = (float)GetScreenHeight();
    // NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)
    Object *o = ObjectFactory("knight", (Vector2){(float)screenWidth/2, (float)screenHeight}, (Vector2){120.0f, 80.0f});
    Object *skeleton = ObjectFactory("skeleton", (Vector2){(float)screenWidth / 3, (float)screenHeight /2}, (Vector2){64.0f, 48.0f});
    Object *curObj = o;
    Rectangle r0 = {.x = 0, .y = GetScreenHeight() - 140.0f, .width = 180.0f, .height = 140.0f};
    Rectangle r1 = {.x = GetScreenWidth() - 110.0f, .y = GetScreenHeight() - 400.0f, .width = 110.0f, .height = 400.0f};
    Rectangle r2 = {.x = GetScreenWidth() - 110.0f * 2, .y = GetScreenHeight() - 180.0f, .width = 110.0f, .height = 180.0f};
    Rectangle rs[] = {r0, r1, r2};
    Camera2D camera = {0};
    camera.target = (Vector2){o->pos.x + o->c.width / 2, o->pos.y + o->c.height / 2};
    camera.offset = (Vector2){screenWidth / 2, screenHeight - 128.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
    SetTargetFPS(60);             // Set our game to run at 60 frames-per-second
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
        if (IsKeyPressed(KEY_B)) {
            curObj->setBounds = !curObj->setBounds;
        }
        if (IsKeyPressed(KEY_N) && curObj->setBounds) {
            FrameAdvance(curObj->sprite);
        }
        if (IsKeyPressed(KEY_ENTER)) {
            SaveObjectParams(curObj);
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
        EnvCollision(o, &rs, 3);
        Update(o);
        Update(skeleton);
        if (o->c.y + o->c.height < maxHeight) {
            maxHeight = o->c.y + o->c.height;
        }
        camera.target = (Vector2){o->pos.x + o->c.width / 2, o->pos.y + o->c.height /2};
        // sprintf(info, "posx: %.3f | posy: %.3f | recx: %.3f | recy: %.3f | recw: %.3f | recy: %.3f", 
        //     o->pos.x, o->pos.y, o->c.x, o->c.y, o->c.width, o->c.height);
        // sprintf(info, "%s: scale: %.3f, colx: %.3f, coly: %.3f, offsetx: %.3f", 
        //     curObj->name, curObj->spriteScale, curObj->dscale.x, curObj->dscale.y, curObj->offsetX);
        sprintf(info, "xmin: %.3f, xmax: %.3f, ymin: %.3f, ymax: %.3f", 
            o->ob.xmin, o->ob.xmax, o->ob.ymin, o->ob.ymax);
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(DARKBLUE);
            DrawText(info, 2, 60, 40, WHITE);
            // DrawText("HOLD L_CTRL AND USE ARROWS TO SCALE SPRITE", 2, 60 + 40 + 2, 40, WHITE);
            // DrawText("HOLD L_ALT AND USE ARROWS TO SCALE COLLIDER", 2, 60 + 40 * 2 + 2, 40, WHITE);
            // DrawText("HOLD L_SHIFT AND USE ARROWS TO OFFSET COLLIDER", 2, 60 + 40 * 3 + 2, 40, WHITE);
            BeginMode2D(camera);
                Draw(o);
                Draw(skeleton);
                for (int i = 0; i < 3; ++i) {
                    DrawRectangleRec(rs[i], WHITE);
                }
            EndMode2D();
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