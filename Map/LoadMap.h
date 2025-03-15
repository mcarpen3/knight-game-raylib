#ifndef LOADMAP_H
#define LOADMAP_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <raylib.h>
#include <string.h>
#include "../Sprite/sprite.h"
#include "../Object/mapobject.h"

void LoadMap(DList *objList, Rectangle *world);

#endif // LOADMAP_H