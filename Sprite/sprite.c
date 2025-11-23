#include "sprite.h"

void DestroyCollider(Collider *collider)
{
    printf("INFO: Freeing %s collider\n", collider->type == LineType ? "line" : "rect");
    free(collider);
}

void UpdateSprite(SpriteRect *sprite) {
    if (sprite->frameCount <= 1) {
        return;
    }
    sprite->frameCounter++;
    if (sprite->frameCounter > (60 / sprite->frameCount)) {
        sprite->frameCounter = 0;
        sprite->frameIdx++;
        sprite->curCldrEl = sprite->curCldrEl->next == NULL ? 
            list_head(sprite->colliders) :
            sprite->curCldrEl->next;
    }
    if (sprite->frameIdx >= sprite->frameCount) {
        sprite->frameIdx = 0;
    }
}

void DrawSprite(SpriteRect *sprite, Vector2 position, float scale, float rotation, bool reverse) {
    Rectangle dr = (Rectangle){
        .x = sprite->r.x + (sprite->r.width / sprite->frameCount * sprite->frameIdx),
        .y = sprite->r.y,
        .width = reverse ? -(sprite->r.width / sprite->frameCount) : sprite->r.width / sprite->frameCount,
        .height = sprite->r.height,
    };
    Rectangle dest = (Rectangle) {
        .x = position.x - sprite->r.width * scale / sprite->frameCount / 2,
        .y = position.y - sprite->r.height * scale / 2,
        .width = sprite->r.width / sprite->frameCount * scale,
        .height = sprite->r.height * scale,
    };
    Vector2 origin = (Vector2){0};
    DrawTexturePro(sprite->texture, dr, dest, origin, rotation, WHITE);
    Collider *cldr = (Collider *)sprite->curCldrEl->data;
    if (cldr->type == RectType) {
        Rectangle cr = (Rectangle) {
            .x = dest.x + cldr->data.rect.x * scale,
            .y = dest.y + cldr->data.rect.y * scale,
            .width = cldr->data.rect.width * scale,
            .height = cldr->data.rect.height * scale,
        };
        DrawRectangleLinesEx(cr, 1.0f, GREEN);
    } else if (cldr->type == LineType) {
        Line tmp = (Line) {
            .p1 = (Vector2){
                .x = dest.x + cldr->data.line.p1.x * scale, 
                .y = dest.y + cldr->data.line.p1.y * scale},
            .p2 = (Vector2) {
                .x = dest.x + cldr->data.line.p2.x * scale,
                .y = dest.y + cldr->data.line.p2.y * scale,
            }
        };
        DrawLineBezier(tmp.p1, tmp.p2, 1.0f, GREEN);
    }
}

void DestroySprite(SpriteRect *sr)
{
    printf("INFO: SpriteRect %s destroying\n", sr->name);
    list_destroy(sr->colliders);
    free(sr->filename);
    free(sr->name);
    free(sr);
}