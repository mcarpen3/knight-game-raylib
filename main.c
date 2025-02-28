#include "raylib.h"
#include "./object/object.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "experiments/LoadWorld.c"
#include "datastructs/linkedlist.h"

#define MAX_FRAME_SPEED     15
#define MIN_FRAME_SPEED      1
#define ARRAY_LEN(x)(sizeof(x) / sizeof((x)[0]))
#define CAM_SCALE_Y         1.4f
#define BG_OFF_SCALAR       0.4f
#define SCREEN_WIDTH        1280
#define SCREEN_HEIGHT       720
#define WORLD_WIDTH         SCREEN_WIDTH * 4
static void DestroySpriteRects(SpriteRect *spriteRect) {
    UnloadTexture(spriteRect->texture);
    free(spriteRect->filename);
    free(spriteRect);
}
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    srand(time(NULL));
    const float groundHeight = 128.0f;
    char *info = (char *)malloc(256);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Knight HERO game!!!!");
    // NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)
    Object *o = ObjectFactory("knight", (Vector2){(float)SCREEN_WIDTH/2, (float)SCREEN_HEIGHT}, (Vector2){120.0f, 80.0f});
    Object *skeleton = ObjectFactory("skeleton", (Vector2){(float)SCREEN_WIDTH / 3, (float)SCREEN_HEIGHT /2}, (Vector2){64.0f, 48.0f});
    Object *curObj = o;
    List *envList = (List *)malloc(sizeof(List));
    List *envColliderLines = (List *)malloc(sizeof(List));
    List *envColliderRects = (List *)malloc(sizeof(List));
    list_init(envList, (void *)DestroySpriteRects);
    LoadWorld(envList, envColliderLines, envColliderRects);
    Texture2D bg = LoadTexture("./spritesheets/background/background.png");
    Camera2D camera = {0};
    camera.target = (Vector2){o->pos.x + o->c.width / 2, o->pos.y + o->c.height / 2};
    camera.offset = (Vector2){SCREEN_WIDTH / 2, SCREEN_HEIGHT - groundHeight};
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
        // EnvCollision(o, rs, 3);
        EnemyCollision(o, skeleton);
        Update(o);
        Update(skeleton);

        if (o->pos.x <= SCREEN_WIDTH / 2) {
            camera.target = (Vector2){SCREEN_WIDTH / 2, SCREEN_HEIGHT - groundHeight};
        } else {
            camera.target = (Vector2){o->pos.x, SCREEN_HEIGHT - groundHeight};
            if (o->vel.x > 0) {
                bgOffset -= BG_OFF_SCALAR;
            } else if (o->vel.x < 0) {
                bgOffset += BG_OFF_SCALAR;
            }
        }
        sprintf(info, "vel: y: %f, jmp: %d", o->vel.y, o->jc);
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(DARKBLUE);
            // DrawText("HOLD L_CTRL AND USE ARROWS TO SCALE SPRITE", 2, 60 + 40 + 2, 40, WHITE);
            // DrawText("HOLD L_ALT AND USE ARROWS TO SCALE COLLIDER", 2, 60 + 40 * 2 + 2, 40, WHITE);
            // DrawText("HOLD L_SHIFT AND USE ARROWS TO OFFSET COLLIDER", 2, 60 + 40 * 3 + 2, 40, WHITE);
            float bgXIdx = 0;
            while(bgXIdx <= WORLD_WIDTH) {
                Rectangle bgrec = (Rectangle){0, 0, bg.width, bg.height};
                Rectangle bgrecDest = (Rectangle){bgXIdx + bgOffset, 0, SCREEN_WIDTH, SCREEN_HEIGHT - groundHeight};
                DrawTexturePro(bg, bgrec, bgrecDest, (Vector2){0.0f, 0.0f}, 0.0f, WHITE);
                bgXIdx += SCREEN_WIDTH;
            }
            BeginMode2D(camera);
                Draw(o);
                Draw(skeleton);
                ListElmt *envItems = envList->head;
                while(envItems != NULL) {
                    SpriteRect *tmp = (SpriteRect *)envItems->data;
                    DrawTexturePro(tmp->texture, tmp->src, tmp->dest, (Vector2){
                        .x = tmp->dest.width / 2,
                        .y = tmp->dest.height / 2
                    }, tmp->rotation, WHITE);
                    envItems = envItems->next;
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