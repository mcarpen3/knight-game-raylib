#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "Map/LoadMap.h"
#include "World/TextureSource.h"

#define MAX_FRAME_SPEED     15
#define MIN_FRAME_SPEED      1
#define ARRAY_LEN(x)(sizeof(x) / sizeof((x)[0]))
#define CAM_SCALE_Y         1.4f
#define BG_OFF_SCALAR       0.4f
#define SCREEN_WIDTH        1280
#define SCREEN_HEIGHT       720

int main(void)
{
    // Initialization
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Knight HERO game!!!!");
    // NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)
    DList *objList = (DList *)malloc(sizeof(DList));
    Set *textures = (Set *)malloc(sizeof(Set));
    set_init(textures, &matchTexture, (void *)DestroyTextureSource);
    dlist_init(objList, (void *)DestroyObject);
    Rectangle world = (Rectangle){0};
    printf("INFO: ----- LOADMAP -----\n");
    LoadMap(objList, &world);
    printf("INFO: ----- END LOADMAP -\n");
    DListElmt *objEl = dlist_head(objList);
    Object *playerObj;
    char *info = (char *)malloc(64);

    // populate the world!!!!!
    while (objEl != NULL) {
        printf("INFO: OBJECT: Loading %s\n", ((Object *)objEl->data)->name);
        if (((Object *)objEl->data)->type == Player) {
            playerObj = (Object *)objEl->data;
        }
        DListElmt *sprtEl = dlist_head(((Object *)objEl->data)->sprites);
        while (sprtEl != NULL) {
            DListElmt *textureEl = dlist_head(textures);
            TextureSource *ts;
            ts = (TextureSource *)malloc(sizeof(TextureSource));
            ts->filename = strdup(((SpriteRect *)sprtEl->data)->filename);
            if (set_insert(textures, ts) != -1) {
                ts->texture = LoadTexture(ts->filename);
                ((SpriteRect *)sprtEl->data)->texture = ts->texture;
            } else {
                free(ts);
                // find the texture
                while (textureEl != NULL) {
                    char *file1 = ((TextureSource *)textureEl->data)->filename;
                    char *file2 = ((SpriteRect *)sprtEl->data)->filename;
                    if (!(strcmp(file1, file2))) {
                        ((SpriteRect *)sprtEl->data)->texture = ((TextureSource *)textureEl->data)->texture;
                        break;
                    }
                }
            }
            sprtEl = sprtEl->next;
        }
        objEl = objEl->next;
    }
    Camera2D camera = {0};
    camera.target = playerObj->position;
    camera.offset = (Vector2){SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
    camera.rotation = 0.0f;
    camera.zoom = 0.5f;
    printf("INFO: PLAYER: {%.2f, %.2f}\n", playerObj->position.x, playerObj->position.y);
    Collider *curCldr = (Collider *)((SpriteRect *)playerObj->curSpriteEl->data)->curCldrEl->data;
    printf("INFO: SPRTRECT: {%.2f, %.2f, %.2f, %.2f}\n", curCldr->data.rect.x, curCldr->data.rect.y, curCldr->data.rect.width, curCldr->data.rect.height);
    SetTargetFPS(60);             // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        sprintf(info, "%.2f, %.2f, vel: %.2f\n", playerObj->position.x, playerObj->position.y, playerObj->vel.x);
        // Update
        //----------------------------------------------------------------------------------
        UpdateMapObject(playerObj);
        camera.target = playerObj->position;
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(DARKBLUE);
            BeginMode2D(camera);
            DrawRectangleRec(world, BROWN);
            DrawMapObject(playerObj);
            for (objEl = dlist_head(objList); objEl != NULL; objEl = dlist_next(objEl)) {
                DrawMapObject((Object *)objEl->data);
            }
            DrawCircleV(playerObj->position, 10.0f, RED);
            EndMode2D();
            DrawText(info, 0, 0, 30.0f, WHITE);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }
    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();                // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    return 0;
}