#include <raylib.h>
#define W 1920
#define H 1080
int main(void) {
    InitWindow(W, H, "DrawTextPro Test!");
    Vector2 center = (Vector2) {
        .x = W / 2,
        .y = H / 2,
        // Draw Crosshairs
    };
    const float fontsize = 30.0f;
    Vector2 fontvec = MeasureTextEx(GetFontDefault(), "Hello, World", fontsize, 1.0f);
    while(!WindowShouldClose()) {
        
        BeginDrawing();
        DrawLine(W / 2, 0, W / 2, H, GREEN);
        DrawLine(0, H / 2, W, H / 2, GREEN);
        DrawRectangleLines(center.x, center.y, fontvec.x, fontvec.y, GREEN);
        DrawTextPro(GetFontDefault(), "Hello, World", center, (Vector2){0}, 0.0f, 30.f, 1.0f, GREEN);
        EndDrawing();
    }
}