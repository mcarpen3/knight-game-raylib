#include <raylib.h>
#include <stdio.h>
#include "../datastructs/linkedlist.h"
#include "../object/mapobject.h"

void DestroyRect(Rectangle *r) {
    free(r);
};

void GetSpriteBounds(Image img, List *rects) { 
    int minx;
    int yAlphaSum = 0;
    bool minxFound = false;
    for (int w = 0; w < img.width; ++w) {
        // iterate horizontally
        for (int h = 0; h < img.height; ++h) {
            // iterate vertically
            Color c = GetImageColor(img, w, h);
            yAlphaSum += c.a;
        }
        if (yAlphaSum > 0 && !minxFound) {
            minx = w;
            minxFound = true;
        }
        if (yAlphaSum == 0 && minxFound) {
            Collider *collider = (Collider *)malloc(sizeof(Collider));
            collider->type = RectType;
            collider->data.rect = (Rectangle){
                .x = minx,
                .y = 0,
                .width = w - minx - 1,
                .height = 0
            };
            list_ins_next(rects, list_tail(rects), collider);
            minxFound = false;
        }
        yAlphaSum = 0;
    }
    ListElmt *idx = list_head(rects);
    while(idx != NULL) {
        int xAlphaSum = 0;
        bool minyFound = false;
        Collider *tmp = (Collider *)idx->data;
        for (int h = 0; h < img.height; ++h) {
            for (int w = tmp->data.rect.x; w < tmp->data.rect.x + tmp->data.rect.width; ++w) {
                Color c = GetImageColor(img, w, h);
                xAlphaSum += c.a;
            }
            if (xAlphaSum > 0 && !minyFound) {
                tmp->data.rect.y = h;
                minyFound = true;
            }
            if (minyFound && (xAlphaSum == 0 || h == img.height - 1)) {
                tmp->data.rect.height = h - tmp->data.rect.y;
                break;
            }
            xAlphaSum = 0;
        }
        idx = idx->next;
    }
}