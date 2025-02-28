#include <raylib.h>

int main(void) {
    List *rects = (List *)malloc(sizeof(List));
    list_init(rects, (void *)DestroyRect);
    InitWindow(1920, 1080, "AutoGetSpriteRect");
    Image img = LoadImage("/home/matt/Documents/c/raylib/spritesheets/knight/_WallClimb.png");
    GetSpriteBounds(img, rects);
    Texture2D texture = LoadTextureFromImage(img);
    ListElmt *el;
    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(DARKGRAY);
        DrawTexture(texture, 0, 0, WHITE);
        el = list_head(rects);
        while(el != NULL) {
            Rectangle *r = (Rectangle *)el->data;
            DrawRectangleLinesEx(*r, 1.0f, GREEN);
            el = el->next;
        }
        EndDrawing();
    }
}