#include "raylib.h"
#include "./object/object.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define MAX_FRAME_SPEED     15
#define MIN_FRAME_SPEED      1
#define ARRAY_LEN(x)(sizeof(x) / sizeof((x)[0]))
#define CAM_SCALE_Y         1.4f
#define BG_OFF_SCALAR       0.4f

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    srand(time(NULL));
    const int screenWidth = 1920;
    const int screenHeight = 1080;
    const float groundHeight = 128.0f;
    const float end = screenWidth * 2;
    char *info = (char *)malloc(256);

    InitWindow(screenWidth, screenHeight, "Knight HERO game!!!!");
    // NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)
    Object *o = ObjectFactory("knight", (Vector2){(float)screenWidth/2, (float)screenHeight}, (Vector2){120.0f, 80.0f});
    Object *skeleton = ObjectFactory("skeleton", (Vector2){(float)screenWidth / 3, (float)screenHeight /2}, (Vector2){64.0f, 48.0f});
    Object *curObj = o;
    Rectangle r0 = {.x = 0, .y = GetScreenHeight() - 140.0f, .width = 180.0f, .height = 140.0f};
    Rectangle r1 = {.x = GetScreenWidth() - 110.0f, .y = GetScreenHeight() - 180.0f, .width = 110.0f, .height = 180.0f};
    Rectangle r2 = {.x = GetScreenWidth() - 110.0f * 2, .y = GetScreenHeight() - 180.0f, .width = 110.0f, .height = 180.0f};
    Rectangle rs[] = {r0, r1, r2};
    Texture2D bg = LoadTexture("./spritesheets/background/background.png");
    Camera2D camera = {0};
    camera.target = (Vector2){o->pos.x + o->c.width / 2, o->pos.y + o->c.height / 2};
    camera.offset = (Vector2){screenWidth / 2, screenHeight - groundHeight};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
    float bgOffset = 0.0f;
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
            ShiftColliderOffset(curObj);
        } else {
            if (IsKeyDown(KEY_RIGHT)) {
                Forward(curObj);
            }
            else if (IsKeyDown(KEY_LEFT)) {
                Backward(curObj);
            } 
            else Decel(curObj);
            if (IsKeyDown(KEY_UP)) Jump(curObj);
            if (IsKeyDown(KEY_SPACE)) Attack(curObj);
            if (IsKeyUp(KEY_SPACE) && IsKeyUp(KEY_UP)) {
                Rest(curObj);
            }
        }
        EnvCollision(o, rs, 3);
        EnemyCollision(o, skeleton);
        Update(o);
        Update(skeleton);

        if (o->pos.x <= screenWidth / 2) {
            camera.target = (Vector2){screenWidth / 2, screenHeight - groundHeight};
        } else {
            camera.target = (Vector2){o->pos.x, screenHeight - groundHeight};
            if (o->vel.x > 0) {
                bgOffset -= BG_OFF_SCALAR;
            } else if (o->vel.x < 0) {
                bgOffset += BG_OFF_SCALAR;
            }
        }
        // sprintf(info, "posx: %.3f | posy: %.3f | recx: %.3f | recy: %.3f | recw: %.3f | recy: %.3f", 
        //     o->pos.x, o->pos.y, o->c.x, o->c.y, o->c.width, o->c.height);
        // sprintf(info, "%s: scale: %.3f, colx: %.3f, coly: %.3f, offsetx: %.3f", 
        //     curObj->name, curObj->spriteScale, curObj->dscale.x, curObj->dscale.y, curObj->offsetX);
        // sprintf(info, "posx: %.2f, posy: %.2f, velx: %.2f, vely: %.2f", o->pos.x, o->pos.y, o->vel.x, o->vel.y);
        sprintf(info, "vel: y: %f, jmp: %d", o->vel.y, o->jc);
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(DARKBLUE);
            // DrawText("HOLD L_CTRL AND USE ARROWS TO SCALE SPRITE", 2, 60 + 40 + 2, 40, WHITE);
            // DrawText("HOLD L_ALT AND USE ARROWS TO SCALE COLLIDER", 2, 60 + 40 * 2 + 2, 40, WHITE);
            // DrawText("HOLD L_SHIFT AND USE ARROWS TO OFFSET COLLIDER", 2, 60 + 40 * 3 + 2, 40, WHITE);
            float bgXIdx = 0;
            while(bgXIdx <= end) {
                Rectangle bgrec = (Rectangle){0, 0, bg.width, bg.height};
                Rectangle bgrecDest = (Rectangle){bgXIdx + bgOffset, 0, screenWidth, screenHeight - groundHeight};
                DrawTexturePro(bg, bgrec, bgrecDest, (Vector2){0.0f, 0.0f}, 0.0f, WHITE);
                bgXIdx += screenWidth;
            }
            BeginMode2D(camera);
                Draw(o);
                Draw(skeleton);
                for (int i = 0; i < 3; ++i) {
                    DrawRectangleRec(rs[i], WHITE);
                }
            EndMode2D();
            DrawText(info, 2, 60, 40, WHITE);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }
    // De-Initialization
    //--------------------------------------------------------------------------------------
    Destroy(o);
    UnloadTexture(bg);
    CloseWindow();                // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    return 0;
}